//Build and send a Publish message for a client.

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Client_t.h"
#include "MQTTSNPublish.h"
#include "MQTTSNPacket.h"
#include "ErrorCodes.h"
#include "Publish.h"
#include "transport.h"
#include "PubRecRelComp.h"
#include "StackTrace.h"

/**
 *  Builds and sends out a Publish message for a client containing the specified data.
 * @param cltPtr A pointer to the client who will be publishing a message.
 * @param flags A pointer to a MQTTSNFlags struct that will contain the message flags set to the appropriate values.
 * @param topicID The topic Id that this message will be tied to.
 * @param msgID The ID of the message being sent.
 * @param data The actual data contained within the message.
 * @return An int: Q_NO_ERR (0) indicates no error for building and sending the message. Otherwise, 
 */

int publish(Client_t *clientPtr, MQTTSNFlags *flags, uint16_t topicID, uint16_t msgID, unsigned char *data)
{
    int returnCode = Q_ERR_Unknown;
    //Number of bytes needed in the buffer.
    size_t bufBytes = 0;
    //Length of the buffer that will hold the data of the message in bytes
    size_t bufSize = 0;
    //Length of the serialized message.
    size_t serialLength = 0;

    MQTTSN_topicid topic;

    FUNC_ENTRY;
    //Length of the message being sent.
    size_t dataLength = strlen((char *)data);

    //MsgID is only considered for QoS levels 1 and 2 
    if(flags->bits.QoS == 0b00){
        msgID = 0;
    }

    //Check if this is a normal topic ID
    if(flags->bits.topicIdType == 0b00) {
        topic.type = MQTTSN_TOPIC_TYPE_NORMAL;
        topic.data.id = topicID;
    }
    //Otherwise, check if this is a pre-defined topic ID
    else if(flags->bits.topicIdType == 0b01) {
        topic.type = MQTTSN_TOPIC_TYPE_PREDEFINED;
        topic.data.id = topicID;
    }
    //Otherwise, check if this is a short topic name
    else if(flags->bits.topicIdType == 0b10) {
        topic.type = MQTTSN_TOPIC_TYPE_SHORT;
        topic.data.short_name[1] = (char)topicID;
    }
    //If not one of the above values, this is not an accepted value for the topicIdType.
    //We must do the goto statement after the buffer has been created, otherwise the compiler will 
    //give a warning
    else {
        returnCode = Q_ERR_TopicIdType;
    }
    //Obtain the packet length so we know how large the buffer needs to be.
    bufBytes = MQTTSNPacket_len(MQTTSNSerialize_publishLength(dataLength, topic, flags->bits.QoS));
    //Create the buffer that will be sent out and contains the entire packet.
    unsigned char buf[bufBytes];
    //Obtain the size of the buffer for serializing the message.
    bufSize = sizeof(buf);

    if(returnCode == Q_ERR_TopicIdType){
        goto exit;  
    }

    //Serialize the message with the given information and check the return code.
    returnCode = MQTTSNSerialize_publish(buf, bufSize, flags->bits.dup, flags->bits.QoS, flags->bits.retain, msgID, topic, data, dataLength);

    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    //Check if the message was successfully sent to the server.
    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    //If this is a QoS level 2 message, the server should send a PubRec message.
    //The MsgID of the PubRec message should match that of the Publish msg.
    if(flags->bits.QoS == 0b10)
    {
        //New buffer to hold the PubRec, PubRel, and PubComp messages
        unsigned char buf2[1600];
        size_t bufSize2 = sizeof(buf2);

        if(MQTTSNPacket_read(buf2, bufSize2, transport_getdata) == MQTTSN_PUBREC)
        {
            uint8_t msgType = 0;
            uint16_t pubRecMsgID = 0;

            if(MQTTSNDeserialize_ack(&msgType, &pubRecMsgID, buf2, bufSize2) != 1){
                returnCode = Q_ERR_Deserial;
                goto exit;
            }

            if(pubRecMsgID != msgID){
                    returnCode = Q_ERR_MsgID;
                    goto exit;
            }

            //In response to a PUBREC, we must send out a PUBREL
            returnCode = pubRecRelComp(clientPtr, msgID, MQTTSN_PUBREL);

            //If the message was not successfully sent.
            if(returnCode != Q_NO_ERR){
                returnCode = Q_ERR_PubRel;
                goto exit;
            }

            //The server should respond with a PUBCOMP message.
            if(MQTTSNPacket_read(buf2, bufSize2, transport_getdata) == MQTTSN_PUBCOMP)
            {
                //Deserialize the message and make sure the msgID matches
                if(MQTTSNDeserialize_ack(&msgType, &pubRecMsgID, buf2, bufSize2) != 1)
                {
                    returnCode = Q_ERR_Deserial;
                    goto exit;
                }

                if(pubRecMsgID != msgID)
                {
                    returnCode = Q_ERR_MsgID;
                    goto exit;
                }

                returnCode = Q_NO_ERR;
                goto exit;
            }
            //The client needs to resend a pubRel message. 
            else
            {
                returnCode = Q_ERR_SendRel;
                goto exit;
            }
            
        }
        //If a PubRec message has not been received, the status code will indicate
        //the message needs to be resent.
        else
        {
            returnCode = Q_ERR_RePub;
            goto exit;
        }
    }//End if statement for QoS level 2

    //If this is a QoS level 1, the server will send back a PubAck
    //message, which should be checked for its return code.
    //TODO, if the server sends a return code of congested, add functionality for sending after Twait time.
    else if(flags->bits.QoS == 0b01)
    {
        //New buffer to hold the PubAck message.
        unsigned char buf2[1600];
        size_t bufSize2 = sizeof(buf2);
        if(MQTTSNPacket_read(buf2, bufSize2, transport_getdata) == MQTTSN_PUBACK)
        {
            uint8_t returnCode3 = 0;
            uint16_t ackTopicID = 0;
            uint16_t ackMsgID = 0;

            //Ensure the message was deserialized successfully and it has a return code of accepted.
            if(MQTTSNDeserialize_puback(&ackTopicID, &ackMsgID, &returnCode3, buf2, bufSize2) != 1) 
            {
                returnCode = Q_ERR_Deserial;
                goto exit;
            }

            //Check if the "accepted" return code was sent back from the server.
            else if(returnCode3 != MQTTSN_RC_ACCEPTED)
            {  
                returnCode = Q_ERR_MsgReturnCode;
                goto exit;
            }

            //Check if the message ID of the acknowledgment message matches the message ID of the publish message
            //that was sent originally.
            else if(ackMsgID != msgID)
            {
                returnCode = Q_ERR_MsgID;
                goto exit;
            }
            
            else
            {
                returnCode = Q_NO_ERR;
                goto exit;
            }
        }
        else
        {
            returnCode = Q_ERR_PubAck;
            goto exit;
        }
        
    }//End if statement for QoS level 1
    
    //For QoS level 0 there is no need to check for a PubAck message from the server.
    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
