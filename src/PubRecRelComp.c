//Build and send a PubRec, PubRel, or PubComp message

#include <stdint.h>
#include <stdlib.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNPublish.h"
#include "ErrorCodes.h"
#include "PubRecRelComp.h"
#include "StackTrace.h"
#include "transport.h"

/**
 * The following function builds and sends a PubRec, PubRel, or PubComp message
 * for a client depending on which message is chosen by the msgType parameter.
 * @param clientPtr The client who will be sending the message
 * @param msgID The corressponding publish message ID
 * @param msgType Indicates the message type so the program knows which to construct. 
 * Can be MQTTSN_PUBREL (16), MQTTSN_PUBREC (15), or MQTTSN_PUBCOMP (14).
 * 
 * @return An int: Q_NO_ERR(0) indicates success. Otherwise: Q_ERR_Unknown(22), Q_ERR_MsgType (20),
 *  Q_ERR_Socket (1) indicate an error.
 * 
 */
int pubRecRelComp(Client_t *clientPtr, uint16_t msgID, enum MQTTSN_msgTypes msgType)
{
    int returnCode = Q_ERR_Unknown;

    //The buffer to hold the message.
    unsigned char buf[4];

    //The size of the buffer.
    size_t bufSize = sizeof(buf);

    //The length of the serialized message
    size_t serialLength = 0;

    FUNC_ENTRY;

    if(msgType > MQTTSN_PUBREL || msgType < MQTTSN_PUBCOMP){
        returnCode = Q_ERR_MsgType;
        goto exit;
    }

    //Check which message type of PubRec, PubRel, PubComp is being sent out and serialize that message.
    if(msgType == MQTTSN_PUBCOMP){
        returnCode = MQTTSNSerialize_pubcomp(buf, bufSize, msgID);
    }

    else if(msgType == MQTTSN_PUBREC){
        returnCode = MQTTSNSerialize_pubrec(buf, bufSize, msgID);
    }

    else{
//Debug
puts("PUBREL");
        returnCode = MQTTSNSerialize_pubrel(buf, bufSize, msgID);
    }
    

    if(returnCode > 0){
        serialLength = (size_t) serialLength;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr-> destinationPort, buf, serialLength);

    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
