//Build and send a PubAck message

#include <stdlib.h>
#include <stdint.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNPublish.h"
#include "ErrorCodes.h"
#include "PubAck.h"
#include "StackTrace.h"
#include "transport.h"

/**
 * Builds and sends out a PubAck message in response to a pubish message. 
 * This is used in response to receiving a Publish message with QoS level 1.
 * @param clientPtr The client who will be sending out the PubAck message
 * @param topicID The topicID used in the corresponding Publish message.
 * @param msgID The message ID used in the corresponding Publish message.
 * @param msgReturnCode Indicates the corresponding Publish message is either accepted or rejected.
 * @return An int: Q_NO_ERR indicates success. Otherwise, Q_ERR_Serial and Q_ERR_Socket indicate an error.
 */ 
int pubAck(Client_t *clientPtr, uint16_t topicID, uint16_t msgID, uint8_t msgReturnCode)
{
    int returnCode = 0;

    //The number of bytes needed in the buffer.
    size_t bufBytes = 6;

    //The buffer the message will be written into.
    unsigned char buf[bufBytes];

    //The size of the buffer.
    size_t bufSize = sizeof(buf);

    //The serialized length of the message.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //Serialize the message.
    returnCode = MQTTSNSerialize_puback(buf, bufSize, topicID, msgID, msgReturnCode);

    //Check if serialization was successful.
    if(returnCode > 0){

        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    //Send out the message.
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    //Ensure that the message was successfully sent.
    if(returnCode2 != 0){

        returnCode = Q_ERR_Socket;
        goto exit;
    }

    returnCode = Q_NO_ERR;

exit: 
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
