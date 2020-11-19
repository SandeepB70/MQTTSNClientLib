/**
 * Build and send the WillMsg packet for a MQTTSN Client.
 */ 

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "ErrorCodes.h"
#include "WillMsg.h"
#include "transport.h"
#include "StackTrace.h"

/**
 * Builds and sends WillMsg message for the specified client.
 * @param clientPtr The MQTTSN client who will be sending out the will message.
 * @param willMsg An MQTTSNString struct that contains the Will message from the client.
 * @return An integer: Q_NO_ERR indicates the WillMsg message was acknowledged by the server/gateway. 
 * Otherwise, Q_ERR_Serial, Q_ERR_Socket, and Q_ERR_Connack indicate an error occurred. 
 */ 
int willMsg(Client_t *clientPtr, MQTTSNString *willMsg)
{
    int returnCode;
    size_t bufBytes = 0;
    //Represnts the length of the serialized version of the packet.
    size_t serialLength = 0;
    //Size of the actual buffer. This will be needed to send out the packet to the server.
    size_t bufSize = 0;

    FUNC_ENTRY;
    //We add 1 to account for the 1 byte taken up by the MsgType portion.
    bufBytes = MQTTSNPacket_len(MQTTSNstrlen(*willMsg) + 1);

    //buffer that will be holding the packet to be sent out
    unsigned char buf[bufBytes];

    bufSize = sizeof(buf);

    //Serialize the message into the buffer (buf).
    returnCode = MQTTSNSerialize_willmsg(buf, bufSize, *willMsg);


    //If 0 was not returned, the serialization of the WillMsg was successful.
    if(returnCode != 0 && returnCode > 0)
    {
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }
    //Send the message out.
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    //Ensure transfer of the packet is successful. 
    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }


returnCode = Q_NO_ERR;
    
exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
