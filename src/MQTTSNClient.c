//The following code specifies the Connect message for an MQTT-SN client.
#include <stddef.h>


#include "/home/sunny/Desktop/Tardigrade/mqtt-sn/MQTTSNPacket/src/MQTTSNConnect.h"
#include "/home/sunny/Desktop/Tardigrade/mqtt-sn/MQTTSNPacket/src/MQTTSNPacket.h"

//Defines the Connect message for an MQTT-SN client according to the MQTT-SN Protocol Specification (Version 1.2).
typdef struct 
{
    unsigned char length;
    unsigned char msgType = 0x04;
    MQTTSNFlags flag;
    unsigned char protcolID = MQTTSN_PROTOCOL_VERSION;
    unsigned int duration;
    char clientID[];
} Connect;

/** 
 * Creates a connect message for an existing client to send to the server. 
 * @param id The Client ID for the existing client.
 * @param idLength The length in bytes of the Client ID.
 * @param flagOptions Represents the flags portion of the connect message with the bits fields set appropriately.
 * @param keepAlive The value for the Keep Alive timer (in seconds).
 * @return 0 indicates successful creation of a message. 1 indicates an invalid Client ID length. 
*/
int createConnectMsg(Connect *message, const char id[], const unsigned char idLength, MQTTSNFlags flagOptions, unsigned int keepAlive) {
    if(idLength > 23 || idLength < 1) {
        puts("Error: Invalid Client ID length.");
        return 1;
    }

    message->clientID = id;
    message->flag = flagOptions;
    message->duration = keepAlive;

} //End createConnectMsg




