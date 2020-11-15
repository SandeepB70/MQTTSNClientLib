//Builds and Sends the WillMsgUpd message.

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "ErrorCodes.h"
#include "Client_t.h"
#include "StackTrace.h"

/**
 * Builds and Sends out a WillMsgUpd and processes the incoming WillMsgResp that should be returned from the Server/GW
 * @param clientPtr The client that will be sending out the message.
 * @param willMsg The new WillMsg for the client.
 * @return An int: Q_NO_ERR (0) indicates success. Otherwise, Q_ERR_Unknown (22), Q_ERR_Serial (12), Q_ERR_Socket (1), 
 * Q_ERR_MsgReturnCode (19), Q_ERR_Deserial (3), Q_ERR_MsgType (20) indicate an error.
 */

int WillMsgUpd(Client_t *clientPtr, const MQTTSNString *willMsg)
{
    int returnCode = Q_ERR_Unknown;

    //Holds the number of bytes needed by the buffer that holds the message
    size_t bufBytes = 0;

    //The size of the buffer.
    size_t bufSize = 0;

    //The length of the serialized message.
    size_t serialLength = 0;

    FUNC_ENTRY;
    bufBytes = MQTTSNPacket_len(MQTTSNstrlen(willMsg));

    //The buffer that will be hold the message.
    unsigned char buf[bufBytes];

    //Obtain the size of the buffer
    bufSize = sizeof(buf);

    //Serialize the message.
    returnCode = MQTTSNSerialize_willmsgupd(buf, bufSize, *willMsg);

    //Check if serialization was successful and assign the length to the serialLength variable
    if(returnCode > 0){
        serialLength = returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    //Send the message out.
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    //Check if the message was successfully sent out.
    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    returnCode = Q_NO_ERR;

/**
 * 
 * OLD CODE
    //Buffer meant to take in a message from the server/GW.
    unsigned char buf2[1600];

    //The size of the new buffer.
    size_t bufSize2 = sizeof(buf2);

    //Client should receive a WillMsgResp message containing a return code.
    //Need to check if it was received and has a return of "accepted".
    if(MQTTSNPacket_read(buf2, bufSize2, transport_getdata) == MQTTSN_WILLMSGRESP)
    {
        //Used to hold the return code contained in the WillMsgResp
        int respRtrnCode = 0;

        //Deserialize the WillMsgResp and check if it was successful.
        if(MQTTSNDeserialize_willmsgresp(&respRtrnCode, buf2, bufSize2) == 1)
        {
            if (respRtrnCode == MQTTSN_RC_ACCEPTED)
            {  
                returnCode = Q_NO_ERR;
                goto exit;
            }
            else
            {
                printf("%s%d\n", "Return Code: ", respRtrnCode);
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
        returnCode = Q_ERR_MsgType;
        goto exit;
    }
*/

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}   

