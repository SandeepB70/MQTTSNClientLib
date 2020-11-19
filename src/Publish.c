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

    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
