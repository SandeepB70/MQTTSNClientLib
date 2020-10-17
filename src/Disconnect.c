//Build and send the disconnect message.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Client.h"
#include "StackTrace.h"
#include "Connect.h"
#include "Disconnect.h"
#include "ErrorCodes.h"

size_t MQTTSNSerialize_disconnectLength(int duration); //prototype for function that is needed.

/**
 * Builds and sends a disconnect message for an MQTTSN Client.
 * @param clientPtr Represents the MQTTSN client who will be sending out he disconnect message.
 * @param sleepTimer The duration value for the Disconnect message, 0 indicates the client wants to disconnect, while 
 *                      any number greater than 0 indicates the number of seconds the client will go to sleep for.
 * @return An int: Q_NO_ERR (0) indicates success and acknowledgement by the server/gateway of the Disconnect message.
 *          Otherwise: Q_ERR_Disconnect (6), Q_ERR_Socket (1), Q_ERR_Deserial (3), Q_ERR_Ack (7) indicate an error.
 */
int disconnect(Client *clientPtr, uint16_t sleepTimer)
{
    int returnCode;
    //Represents the length of the entire MQTTSN message packet in bytes.
    size_t packetLength = 0;
    //Represents the serialized length of the packet.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //Determine the number of elements that will be needed in the buffer.
    packetLength = MQTTSNPacket_len(MQTTSNSerialize_disconnectLength(sleepTimer));
    unsigned char buf[packetLength];
    size_t buflen = sizeof(buf);

    //Serialize the message into the buffer (buf).
    returnCode = MQTTSNSerialize_disconnect(buf, buflen, sleepTimer);

    if (returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Disconnect;
        goto exit;
    }

    //Send the message. 
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
		goto exit;
    }

    //Check for DISCONNECT acknowledgement message.
	if (MQTTSNPacket_read(buf, buflen, transport_getdata) == MQTTSN_DISCONNECT)
	{
		
		//MQTTSNDeserialize_disconnect requires an integer value for the duration field of the packet.
		int duration = sleepTimer; 
		int disconnect_rc = MQTTSNDeserialize_disconnect(&duration, buf, buflen);

		if (disconnect_rc != 1)
		{
			returnCode = Q_ERR_Deserial;
			goto exit;
		}
	}
	else
	{
		returnCode = Q_ERR_Ack;
		goto exit;
	}

    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
