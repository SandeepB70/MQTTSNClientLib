//Test for the disconnect message.

#include <stdlib.h>


#include "Client.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "Disconnect.h"
#include "ErrorCodes.h"

int main(void)
{

    int returnCode;
    //Used for the duration flag of the Connect message
    uint16_t keepAlive = 15;
    //Will be used to set the duration for the disconnect message.
    uint16_t duration = 0;
    //Sets the will flag in the connect message.
    uint8_t willF = 0;
    //Set the clean session flag for the connect message.
    uint8_t clnSession = 1;

    Client testClient;
    testClient.destinationPort = 60885;
    testClient.host = "50.255.7.18";
    testClient.clientID = "SandeepTest";

    returnCode = connect(&testClient, keepAlive, willF, clnSession);

     switch (returnCode) {
        case Q_NO_ERR:
            puts("Success!!");
            break;

        case Q_ERR_CSocket:
            puts("Socket Error");
            break;

        case Q_ERR_Connect:
            puts("Connect Error");
            break;

        case Q_ERR_Deserial:
            puts("Deserial Error");
            break;
        
        case Q_ERR_Connack:
            puts("Connack Error");
            break;
        default:
            puts("Unknown Error");
            break;
    }//End switch

    returnCode = disconnect(&testClient, duration);

    switch (returnCode) {
        case Q_NO_ERR:
            puts("Success!!");
            break;

        case Q_ERR_Disconnect:
            puts("Disconnect Error");
            break;

        case Q_ERR_DSocket:
            puts("Socket Error");
            break;

        case Q_ERR_Deserial:
            puts("Deserial Error");
            break;
        
        case Q_ERR_Ack:
            puts("Disconnect Acknowledgement Error");
            break;
        default:
            puts("Unknown Error");
            break;
    }//End switch

	transport_close();

    return returnCode;
}


