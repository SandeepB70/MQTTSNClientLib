//Build and send the WillTopic message for a MQTTSN Client.
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "StackTrace.h"
#include "Connect.h"
#include "WillTopic.h"
#include "ErrorCodes.h"

//The length of an empty will topic message (2 bytes). Will only be used depending on the TODO section below.
//#define EMPTY_WILL_TOPIC 2

/**
 * Build and send the WillTopic message for a MQTTSN Client
 * @param clientPtr The pointer to the Client struct that should already be connected to a server/gateway.
 * @param flags Represents the flags needed for the WillTopic message.
 * @param willTopic The Will Topic that will be sent to the broker/server.
 * @return An integer: Q_WillMsgReq is a success. Otherwise Q_ERR_QoS, Q_ERR_Retain, Q_ERR_Serial, 
 * Q_ERR_Socket, or Q_ERR_WillMsgReq is returned to indicate an error. 
 */ 
int willTopic(Client_t *clientPtr, MQTTSNFlags flags, MQTTSNString willTopic)
{
    int returnCode;
    size_t bufBytes = 0;
    //Size of the actual buffer. This will be needed to send out the packet to the server.
    size_t bufSize = 0;
    //Represents the length of the serialized version of the packet.
    size_t serialLength = 0;
    /**
     * TODO Ask Lawrence if this is the best way to go about coding an empty Will Topic packet.
     * The code Paho code does not allow for an empty Will Topic message (it always add the flag byte). 
     * The bufSize will always be too short compared to the packet length.
     * 
    if(willTopic.lenstring.len == 0){
        bufBytes = MQTTSNPacket_len(MQTTSNstrlen(willTopic) + 1);
    }
    //The Will Topic is not the empty string so this is 
    else{
        bufBytes = MQTTSNPacket_len(MQTTSNstrlen(willTopic) + 2);
    }
    *
    */
    FUNC_ENTRY;
    bufBytes = MQTTSNPacket_len(MQTTSNstrlen(willTopic) + 2);
    unsigned char buf[bufBytes];
    bufSize = sizeof(buf);

     //Make sure the QoS flag is valid.
    if (flags.bits.QoS > 0b11){
        returnCode = Q_ERR_Qos;
        goto exit;
    }
    //Make sure the Retain flag is valid.
    if(flags.bits.retain > 1){
        returnCode = Q_ERR_Retain;
        goto exit;
    }

    //Serialize the message into the buffer (buf).
    returnCode = MQTTSNSerialize_willtopic(buf, bufSize, flags.bits.QoS, flags.bits.retain, willTopic);

    //Make sure the serialization was a success and assign it to serialLength since
    //the length of the serialized packet gets returned.
    if(returnCode != 0 && returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    //Send out the packet.
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if (returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }


    /**OLD CODE
     * TODO, part of asking Lawrence about an empty will topic message.
    int MQTTSNPacketType = -1;

    if(serialLength == EMPTY_WILL_TOPIC){
        MQTTSNPacketType = EMPTY_WILL_TOPIC;
    }
    printf("\n%d\n", MQTTSNPacketType);
    
    //We now have to check if the server sent a WILLMSGREQ packet back.
    //If it does, we must send a WillMsg back to the server to complete the process.
    if(MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_WILLMSGREQ)
    {
        returnCode = Q_WillMsgReq;
        goto exit;
    }
    else
    {
        returnCode = Q_ERR_WillMsgReq;
        goto exit;
    }
    */

   returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
