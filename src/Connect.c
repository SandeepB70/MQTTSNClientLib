//Contains a function for building the connect message and sending it out for a client.
//Also contains a function called MQTTSNStrCreate for creating a MQTTSNString struct containing
//the specified character string.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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
 * @return An int: Q_NO_ERR(0) indicates success of sending a message with no will flag set. Q_WillTopReq () indicates
 * the connect message was successfully sent and a the server/GW responded with a WillTopicReq message. Otherwise,
 * Q_ERR_Unknown (22), Q_ERR_Socket (1), Q_ERR_SocketOpen (30), Q_ERR_Connect (2), Q_ERR_WillTopReq (5), Q_ERR_MsgReturnCode (19), 
 * Q_ERR_Deserial (3), or Q_ERR_Connack (4).
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
    }
    else {
        returnCode = Q_ERR_Connect;
        goto exit;
    }
	
	//Send the Connect message to the server.
    ssize_t rc = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if (rc != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    //Check if the Will flag has been set to true/on for this connect message.
    //If it is, we need to check if the server is requesting the WillTopic, otherwise something is wrong.
    if(willF == 1)
    {
        //Send a return code status indicating that we need to send out a WillTopic message.
        if(MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_WILLTOPICREQ) 
        {
            returnCode = Q_WillTopReq;
            goto exit;
        }

        else
        {
            returnCode = Q_ERR_WillTopReq;
            goto exit;
        }
    }//end if

    //If the will flag has not been set, check for CONNACK.
	if (MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_CONNACK)
	{
		//Used to hold the return Code of the CONNACK message.
		int connack_rc = -1;
        //Deserialize the message and check if it is successful.
		if (MQTTSNDeserialize_connack(&connack_rc, buf, bufSize) == 1) 
		{
			if(connack_rc == 0)
            {
                returnCode = Q_NO_ERR;
                goto exit;
            }
            else
            {
                returnCode = Q_ERR_MsgReturnCode;
                goto exit;
            }
		}
        else
        {
            returnCode = Q_ERR_Deserial;
			goto exit;
        }
	}
	else
	{
        puts("No connack");
		returnCode = Q_ERR_Connack;
		goto exit;
	}

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End connect
