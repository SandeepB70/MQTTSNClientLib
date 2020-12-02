//Percentage Contribution: Sandeep Bindra (100%)
//Build and send the disconnect message.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "StackTrace.h"
#include "Connect.h"
#include "Disconnect.h"
#include "ErrorCodes.h"

size_t MQTTSNSerialize_disconnectLength(int duration); //prototype for function that is needed.

/**
 * Builds and sends a disconnect message for an MQTTSN Client.
 * @param clientPtr Represents the MQTTSN client who will be sending out he disconnect message.
 * @param sleepTimer The duration value for the Disconnect message, 0 indicates the client wants to disconnect, while 
 * any number greater than 0 indicates the number of seconds the client will go to sleep for.
 * @return An int: Q_NO_ERR indicates success and acknowledgement by the server/gateway of the Disconnect message.
 * Otherwise: Q_ERR_Socket and Q_ERR_Serial indicate an error.
 */
int disconnect(Client_t *clientPtr, uint16_t sleepTimer)
{
    int returnCode;

    //Number of bytes needed in the buffer.
    size_t bufBytes = 0;
    //Represents the serialized length of the packet.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //Determine the number of elements that will be needed in the buffer.
    bufBytes = MQTTSNPacket_len(MQTTSNSerialize_disconnectLength(sleepTimer));
    //buffer to hold the message
    unsigned char buf[bufBytes];
    size_t bufSize = sizeof(buf);

    //Serialize the message into the buffer and check if it was successful.
    returnCode = MQTTSNSerialize_disconnect(buf, bufSize, sleepTimer);

    if (returnCode > 0){
        serialLength = (size_t) returnCode;
    } else {
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    //Send the message. 
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
		goto exit;
    }

    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
