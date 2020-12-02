//Percentage Contribution: Sandeep Bindra (100%)
//Builds and sends out a PingReq message. 

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/time.h>

#include "Client_t.h"
#include "MQTTSNConnect.h"
#include "MQTTSNPacket.h"
#include "ErrorCodes.h"
#include "PingReq.h"
#include "PubAck.h"
#include "StackTrace.h"
#include "transport.h"

/**
 * Following function is to build and send a simple PingReq message. 
 * If a clientID is included (the length of the clientID string is not 0), then this is a sleeping client
 * that will go through the procedure of prompting the Server/GW for any buffered messages.
 * @param clientPtr The client who will be sending out the PingReq message.
 * @param clientID The client ID of sending client. This can be empty (in the form of a clientID with length of 0),
 * or can contain a valid client ID, which will cause the server/GW to check if it has any buffered message for this client.
 * @return An int: Q_NO_ERR for success of a simple ping. 
 * Otherwise, Q_ERR_Unknown, Q_ERR_Serial, and Q_ERR_Socket all indicate an error.
 */

int pingReq(Client_t *clientPtr, MQTTSNString *clientID)
{
    
    int returnCode = Q_ERR_Unknown;

    //The number of bytes needed by the buffer.
    size_t bufBytes = 0;

    //The size of the buffer.
    size_t bufSize = 0;

    //Serialized length of the message.
    size_t serialLength = 0;

    //Checking if we want to specify the client ID for this PingReq message.
    //If we are not including a clientID, the buffer will only need to hold two bytes of data.
    if(clientID->lenstring.len == 0){
        bufBytes = 2;
    } else {
        //Add 1 for the MsgType portion of the message.
        bufBytes = MQTTSNPacket_len(MQTTSNstrlen(*clientID) + 1);
    }

    //The buffer to hold the message.
    unsigned char buf[bufBytes];

    bufSize = sizeof(buf);

    FUNC_ENTRY;
    //Serialize the message
    returnCode = MQTTSNSerialize_pingreq(buf, bufSize, *clientID);
    
    //Check if serialization was successful and if it was, assign the serialized length of the message.
    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    } else {
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if (returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
