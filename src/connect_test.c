//Test for the connect message.

#include <stdlib.h>


#include "Client.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "ErrorCodes.h"

int main(void)
{

    int returnCode;
    //The duration portion for the connect message.
    uint16_t keepAlive = 20;
    //Keep the willflag off since we are just testing the connect message only.
    uint8_t willFlag = 0;
    //The Clean Session flag, which will be set to on.
    uint8_t clnSession = 1;
    Client testClient;
    testClient.destinationPort = 60885;
    testClient.host = "50.255.7.18";
    testClient.clientID = "SandeepTest";

    returnCode = connect(&testClient, keepAlive, willFlag, clnSession);


    switch (returnCode) {
        case Q_NO_ERR:
            puts("Success!!");
            break;

        case Q_ERR_Socket:
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

	transport_close();

    return returnCode;
}


