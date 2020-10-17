/**
 * Build and send the WillMsg packet for a MQTTSN Client.
 */ 

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "Client.h"
#include "ErrorCodes.h"
#include "WillMsg.h"
#include "transport.h"
#include "StackTrace.h"
#include "WillMsg.h"

/**
 * Builds and sends WillMsg message for the specified client.
 * @param clientPtr The MQTTSN client who will be sending out the will message.
 * @param willMsg An MQTTSNString struct that contains the Will message from the client.
 * @return An integer: Q_NO_ERR (0) indicates the WillMsg message was acknowledged by the server/gateway. 
 *          Otherwise, Q_ERR_Serial (12), Q_ERR_Socket (1), and Q_ERR_Connack (4) indicate an error occurred. 
 */ 
int WillMsg(Client *clientPtr, MQTTSNString willMsg)
{
    int returnCode;
    size_t packetLength = 0;
    //Represnts the length of the serialized version of the packet.
    size_t serialLength = 0;
    //Size of the actual buffer. This will be needed to send out the packet to the server.
    size_t bufLen = 0;

    FUNC_ENTRY;
    //We add 1 to account for the 1 byte taken up by the MsgType portion.
    packetLength = MQTTSNPacket_len(MQTTSNstrlen(willMsg) + 1);

    //buffer that will be holding the packet to be sent out
    unsigned char buf[packetLength];

    bufLen = sizeof(buf);

    //Serialize the message into the buffer (buf).
    returnCode = MQTTSNSerialize_willmsg(buf, bufLen, willMsg);


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


    //Check if the server successfully received the WillMsg by checking for a CONNACK message.
    if(MQTTSNPacket_read(buf, bufLen, transport_getdata) == MQTTSN_CONNACK)
    {
        returnCode = Q_NO_ERR;
        goto exit;
    }
    else
    {
        returnCode = Q_ERR_Connack;
        goto exit;
    }
    
    
exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
