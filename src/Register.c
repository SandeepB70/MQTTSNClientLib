//Percentage Contribution: Sandeep Bindra (20%), Evon Choong (80%)
//Build and send a Register message for a client.

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNPublish.h"
#include "ErrorCodes.h"
#include "Register.h"
#include "transport.h"
#include "StackTrace.h"

/**
 * Builds and sends out a register message for the provided Client.
 * @param clientPtr The client that will be sending out a message.
 * @param msgID Used to identify this particular message and match it with the regack.
 * @param topicname The name of the topic the client is trying to register with the server.
 * @return An int: Q_NO_ERR is a success. Otherwise, Q_ERR_Unknown, Q_ERR_Serial, or Q_ERR_Socket indicate an error. 
 */

int reg(Client_t *clientPtr, uint16_t msgID, MQTTSNString *topicname)
{   
    int returnCode = Q_ERR_Unknown;

    //Since this is being sent by the client, it must be 0.
    //This variable will also be used later to store the returned topicID 
    //from the server if registration is successful.
    uint16_t topicID = 0;

    //Number of bytes needed in the buffer.
    size_t bufBytes = 0;

    //Size of the entire buffer the message is written into.
    size_t bufSize = 0;

    //Size of the serialized message in bytes.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //Get the length of the topic name in order to determine how many bytes the buffer needs to hold.
    size_t topicNameLen = (topicname->cstring) ? strlen(topicname->cstring) : topicname->lenstring.len;

    bufBytes = MQTTSNPacket_len(MQTTSNSerialize_registerLength(topicNameLen));

    //buffer that will hold the Register message
    unsigned char buf[bufBytes];
    bufSize = sizeof(buf);

    returnCode = MQTTSNSerialize_register(buf, bufSize, topicID, msgID, topicname);
    
    //Check if serialization of the message was successful, in which case it returns the length of the serialized message.
    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    } else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    //Check that the message was successfully sent to the server.
    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

   returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
