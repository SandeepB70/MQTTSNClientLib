//Build and Send the Subscribe message

#include <stdlib.h>
#include <stdint.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNSubscribe.h"
#include "ErrorCodes.h"
#include "Subscribe.h"
#include "transport.h"
#include "StackTrace.h"

/**
 * Builds and Sends a Subscribe message for a client.
 * @param clientPtr The Client who will be subscribing to a topic.
 * @param topic The topic the client will be subscribing to.
 * @param flags A struct that will contain the appropriate values for the dup, qos, and topicIDType flags.
 * @param msgID The message ID used to identify this particular message for the SubAck message.
 * @return An int: Q_NO_ERR indicates success and Q_WildCard indicates a success and that the server
 * will be sending a Register message since the client subscribed to a topic using a wildcard character. Otherwise,
 * Q_ERR_Unknown, Q_ERR_Socket, Q_ERR_WrongMsgID, Q_ERR_QoS, Q_ERR_MsgReturnCode, 
 * Q_ERR_Deserial, Q_ERR_SubAck indicate an error. 
 */ 
int subscribe(Client_t *clientPtr, MQTTSN_topicid *topic, MQTTSNFlags flags, uint16_t msgID)
{
    int returnCode = Q_ERR_Unknown;

    //Number of bytes needed by the buffer.
    size_t bufBytes = 0;

    //The size of the buffer.
    size_t bufSize = 0;

    //Length of the serialized message.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //Get the number of bytes needed to store the message within the buffer.
    bufBytes = MQTTSNPacket_len(MQTTSNSerialize_subscribeLength(topic));
    //Create the buffer that will store the message.
    unsigned char buf[bufBytes];

    bufSize = sizeof(buf);

    //Ensure the topicIdType is not greater than 2.
    //The subscribe serialize function gets the topicIDType 
    //from the MQTTSN_topicid variable instead of the MQTTSNFlags variable
    if(topic->type > MQTTSN_TOPIC_TYPE_SHORT){
        returnCode = Q_ERR_TopicIdType;
        goto exit;
    }

    if(flags.bits.QoS > 0b10){
        returnCode = Q_ERR_Qos;
        goto exit;
    }

    //Serialize the message and check if it was successful
    returnCode = MQTTSNSerialize_subscribe(buf, bufSize, flags.bits.dup, flags.bits.QoS, msgID, topic);
    
    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    
    //Send the message out and check if it was successful.
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if(returnCode2 != 0)
    {
        returnCode = Q_ERR_Socket;
        goto exit;
    }
/**
    //Wait for a SubAck message
    if(MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_SUBACK)
    {
        //Following variables will store the corressponding values from the deserialized SubAck message
        int ackQos = 0;
        uint16_t ackTopicID = 0;
        uint16_t ackMsgID = 0;
        uint8_t ackReturnCode = 0;

        //Check if deserialization of the message was successful 
        if(MQTTSNDeserialize_suback(&ackQos, &ackTopicID, &ackMsgID, &ackReturnCode, buf, bufSize) == 1)
        {
            
            //Check if the return code is "accepted"
            if(ackReturnCode == MQTTSN_RC_ACCEPTED)
            {

                //Check the QoS level
                if(flags.bits.QoS == ackQos)
                {
                    //Check the MsgID
                    if(ackMsgID == msgID)
                    {
                        //Lastly, check if the client subscribed using a wildcard in the topic name (indicated by 0x0000 for topicID)
                        if(ackTopicID == 0)
                        {
                            returnCode = Q_WildCard;
                            goto exit;
                        }
                        else
                        {
                            clientPtr->topicID = ackTopicID;
                            returnCode = Q_NO_ERR;
                            goto exit;
                        }
                    }
                    else
                    {
                        returnCode = Q_ERR_MsgID;
                        goto exit;
                    }
                }
                else
                {
                    returnCode = Q_ERR_QoS;
                    goto exit;
                }
            }
            else
            {
                printf("\n%s%d\n", "RC: ", ackReturnCode);
                returnCode = Q_ERR_MsgReturnCode;
                goto exit;
            }
        }
        else
        {
            returnCode = Q_ERR_Deserial;
            goto exit;
        }
    }
    else
    {
        returnCode = Q_ERR_SubAck;
        goto exit;
    }
    */
   
   returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
