//Percentage Contribution: Sandeep Bindra (100%)
//Contains a function for building the connect message and sending it out for a client.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Client_t.h"
#include "StackTrace.h"
#include "Connect.h"
#include "ErrorCodes.h"

size_t MQTTSNSerialize_connectLength(MQTTSNPacket_connectData *options); //prototype for a needed function

/**
 * Creates a Connect message for a client and sends it out to the server/gateway.
 * @param clientPtr The client struct that will be sending out the Connect message.
 * @param timeOut The Keep Alive timer for the duration portion of the Connect message.
 * @param willF The value of the will flag, either a 1 or 0.
 * @param clnSession The value of the clean session flag, either a 1 or 0.
 * @return An int: Q_NO_ERR indicates success of sending a message with no will flag set. Q_WillTopReq indicates
 * the connect message was successfully sent and a the server/GW responded with a WillTopicReq message. Otherwise,
 * Q_ERR_Unknown, Q_ERR_Socket, Q_ERR_SocketOpen, and Q_ERR_Serial.
 */
int connect(Client_t *clientPtr, uint16_t timeOut, uint8_t willF, uint8_t clnSession)
{
    int returnCode = Q_ERR_Unknown;
    //Length of the serialized message.
    size_t serialLength = 0;
    //Number of bytes needed in the buffer.
    size_t bufBytes = 0;

    FUNC_ENTRY;
    //Represents the data used to put together this MQTTSN message
    MQTTSNPacket_connectData options = MQTTSNPacket_connectData_initializer;
    options.cleansession = clnSession;
    options.willFlag = willF;
    options.clientID.cstring = clientPtr->clientID;
    options.duration = timeOut;

    
    MQTTSNPacket_connectData *optionsPtr = &options;
    //Obtain the length of the packet that will be sent.
    bufBytes = MQTTSNPacket_len(MQTTSNSerialize_connectLength(optionsPtr));
    //Create the buffer that will hold the MQTTSN message data and be sent out.
    unsigned char buf[bufBytes];
    size_t bufSize = sizeof(buf);
    
    //Create the socket for connection.
    clientPtr->mySocket = transport_open();

    if(clientPtr->mySocket < 0){
        return Q_ERR_SocketOpen;
        goto exit;
    }
    //Establish a connection to the server using the information provided above.
    returnCode = MQTTSNSerialize_connect(buf, bufSize, &options);

    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    }  else {
        returnCode = Q_ERR_Serial;
        goto exit;
    }
	
	//Send the Connect message to the server.
    ssize_t rc = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if (rc != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End connect
