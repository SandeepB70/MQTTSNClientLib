//Builds and Sends a RegAck message for the client


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>


#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNPublish.h"
#include "ErrorCodes.h"
#include "RegAck.h"
#include "transport.h"
#include "StackTrace.h"

/**
 * Builds and sends out a RegAck message meant to be sent from the client. This should only be used when a client
 * reconnects with no CleanSession flag set or when it subscribes to a topic name that has wildcard characters. 
 * @param clientPtr The client who will be sending out the acknowledgment message
 * @param topicID The topicID that should have been sent by the server in its register message.
 * @param msgID The messsage ID that should have been sent by the server.
 * @return An int: Q_NO_ERR indicates success. Otherwise, Q_ERR_Serial, Q_ERR_Socket indicate errors.
 */

int regAck(Client_t *clientPtr, uint16_t topicID, uint16_t msgID, uint8_t msgReturnCode)
{

    int returnCode = Q_ERR_Unknown;

    //The return code we want to set for the RegAck message being sent. Since this is the client, 
    //it will acknowledge that it received and processed the register message with an "accepted" (0) return code.
    uint8_t regAckReturnCode = msgReturnCode;

    //The number of bytes needed in the buffer. Since this is a RegAck message, it will only be 7 bytes.
    size_t bufBytes = 7;

    //buffer to hold the message
    unsigned char buf[bufBytes];

    //The size of the buffer.
    size_t bufSize = sizeof(buf);

    FUNC_ENTRY;
    //The serialized length of the message
    size_t serialLength = 0;

    //Check if the return code provided is within the valid range.
    if(regAckReturnCode > 3){
        returnCode = Q_ERR_Rejected;
        goto exit;
    }

    //Serialize the message and then check the returnCode.
    returnCode = MQTTSNSerialize_regack(buf, bufSize, topicID, msgID, regAckReturnCode);

    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }


    //TODO Ask Lawrence about this. Need to figure out a way to create a buffer large enough
    //to hold the publish message (if it has been sent). May have to also 
    //adjust transport_getdata() so that it receives a flag for recvfrom (MSG_PEEK) or
    //(MSG_WAITALL). 
   
    //If no publish message has been received, then there are no other messages to be expected from the server.
    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
