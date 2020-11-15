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
 * Otherwise: Q_ERR_Disconnect, Q_ERR_Socket, Q_ERR_Deserial, or Q_ERR_Ack indicate an error.
 */
int disconnect(Client_t *clientPtr, uint16_t sleepTimer)
{
    int returnCode;
    
    //time_t timer = 4;

    //Number of bytes needed in the buffer.
    size_t bufBytes = 0;
    //Represents the serialized length of the packet.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //Determine the number of elements that will be needed in the buffer.
    bufBytes = MQTTSNPacket_len(MQTTSNSerialize_disconnectLength(sleepTimer));
    unsigned char buf[bufBytes];
    size_t bufSize = sizeof(buf);

    //Serialize the message into the buffer (buf).
    returnCode = MQTTSNSerialize_disconnect(buf, bufSize, sleepTimer);

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
    /**
     * OLD CODE
    int msgCheck = msgReceived(clientPtr->mySocket, timer);
    if(msgCheck == Q_MsgPending)
    {
        //Check for DISCONNECT acknowledgement message.
	    if (MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_DISCONNECT)
	    {
        
	    	//MQTTSNDeserialize_disconnect requires an integer value for the duration field of the packet.
	    	int duration = sleepTimer; 
	    	int disconnect_rc = MQTTSNDeserialize_disconnect(&duration, buf, bufSize);

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
    }
    */

    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
