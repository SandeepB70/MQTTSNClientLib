//Build and send the WillTopic message for a MQTTSN Client.
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Client.h"
#include "StackTrace.h"
#include "Connect.h"
#include "WillTopic.h"
#include "ErrorCodes.h"

//The length of an empty will topic message (2 bytes). Will only be used depending on the TODO section below.
//#define EMPTY_WILL_TOPIC 2

/**
 * Build and send the WillTopic message for a MQTTSN Client
 * @param clientPtr The pointer to the Client struct that should already be connected to a server/gateway.
 * @param QoS Represents the quality of service level for this Will Topic. Can be 0b00 (level 0), 0b01 (level 1), 
 *              0b10 (level 2), or 0b11 (level 3)
 * @param willRetain Represents the value of the Retain flag for whether or not the server/broker will store the topic.
 * @param willTopic The Will Topic that will be sent to the broker/server.
 * @return An integer: Q_WMR (14) is a success. Otherwise Q_ERR_QoS (10), Q_ERR_Retain (11), Q_ERR_Serial (12), 
 * Q_ERR_Socket (1), or Q_ERR_WMR (15) is returned to indicate an error. 
 */ 
int willTopic(Client *clientPtr, uint8_t willQoS, uint8_t willRetain, MQTTSNString willTopic)
{
    int returnCode;
    size_t packetLength = 0;
    //Size of the actual buffer. This will be needed to send out the packet to the server.
    size_t bufLen = 0;
    //Represents the length of the serialized version of the packet.
    size_t serialLength = 0;
    /**
     * TODO Ask Lawrence if this is the best way to go about coding an empty Will Topic packet.
     * The code Paho code does not allow for an empty Will Topic message (it always add the flag byte). 
     * The bufLen will always be too short compared to the packet length.
     * 
    if(willTopic.lenstring.len == 0){
        packetLength = MQTTSNPacket_len(MQTTSNstrlen(willTopic) + 1);
    }
    //The Will Topic is not the empty string so this is 
    else{
        packetLength = MQTTSNPacket_len(MQTTSNstrlen(willTopic) + 2);
    }
    *
    */
    FUNC_ENTRY;
    packetLength = MQTTSNPacket_len(MQTTSNstrlen(willTopic) + 2);
    unsigned char buf[packetLength];
    bufLen = sizeof(buf);

     //Make sure the QoS flag is valid.
    if (willQoS > 0b11){
        returnCode = Q_ERR_QoS;
        goto exit;
    }
    //Make sure the Retain flag is valid.
    if(willRetain > 1){
        returnCode = Q_ERR_Retain;
        goto exit;
    }

    //Serialize the message into the buffer (buf).
    returnCode = MQTTSNSerialize_willtopic(buf, bufLen, willQoS, willRetain, willTopic);

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

    /**
     * TODO, part of asking Lawrence about an empty will topic message.
    int MQTTSNPacketType = -1;

    if(serialLength == EMPTY_WILL_TOPIC){
        MQTTSNPacketType = EMPTY_WILL_TOPIC;
    }
    printf("\n%d\n", MQTTSNPacketType);
    */
    
    //We now have to check if the server sent a WILLMSGREQ packet back.
    //If it does, we must send a WillMsg back to the server to complete the process.
    if(MQTTSNPacket_read(buf, bufLen, transport_getdata) == MQTTSN_WILLMSGREQ)
    {
        returnCode = Q_WMR;
        goto exit;
    }
    else
    {
        returnCode = Q_ERR_WMR;
        goto exit;
    }

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
