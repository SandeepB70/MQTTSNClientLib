//Percentage Contribution: Sandeep Bindra (100%)
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
 * Builds and Sends out a WillMsgUpd
 * @param clientPtr The client that will be sending out the message.
 * @param willMsg The new WillMsg for the client.
 * @return An int: Q_NO_ERR indicates success. Otherwise, Q_ERR_Unknown, Q_ERR_Serial, or Q_ERR_Socket indicate an error.
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
    } else {
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

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}   

