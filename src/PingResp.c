/**
 * Builds and sends out a PingResp message
 * 
 * 
 */

#include <stddef.h>
#include <sys/types.h>

#include "Client_t.h"
#include "transport.h"
#include "MQTTSNConnect.h"
#include "ErrorCodes.h"
#include "StackTrace.h"

int pingResp(Client_t *clientPtr)
{
    int returnCode = Q_NO_ERR;

    //Only need a two byte buffer for the pingResp message.
    unsigned char buf[2];
    //Size of the buffer
    size_t bufSize = sizeof(buf);
    //Serialized length of the message.
    size_t serialLength = 0;

    //Serialize the message and check if it is successful
    returnCode = MQTTSNSerialize_pingresp(buf, bufSize);
    if(returnCode > 0) {
        serialLength = (size_t)returnCode;
    } else {
        returnCode = Q_ERR_Deserial;
        goto exit;
    }

    //Send out the message and check if it failed to send.
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);
    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }
    //Message has been sent successfully.
    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}

