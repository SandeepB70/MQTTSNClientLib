//Test for the register message.

#include <stdlib.h>


#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "Disconnect.h"
#include "Util.h"
#include "ErrorCodes.h"
#include "Register.h"

int main(void)
{
    int returnCode;
    //The duration portion for the connect message.
    uint16_t keepAlive = 10;
    //Keep the willflag off since we are just testing the connect message only.
    uint8_t willFlag = 0;
    //The Clean Session flag, which will be set to on.
    uint8_t clnSession = 1;
    Client_t testClient;
    testClient.destinationPort = 10000;
    testClient.host = "10.0.2.15";
    testClient.clientID = "SandeepTest";

    returnCode = connect(&testClient, keepAlive, willFlag, clnSession);

    //Check the return code.
    returnCodeHandler(returnCode);

    //The struct that will hold the topic name.
    MQTTSNString topicHolder;

    //The actual topic name.
    char *topicName = "TestingTopic1";

    returnCode = MQTTSNStrCreate(&topicHolder, topicName);

    if(returnCode != Q_NO_ERR){
        puts("Problem with String create");
    }

    //Represents the message ID
    uint16_t msgID = 1;

    returnCode = reg(&testClient, msgID, &topicHolder, &(testClient.topicID));

    //Check the return code.
    returnCodeHandler(returnCode);

    if(returnCode == Q_NO_ERR){
        printf("%s%d\n", "TopicID: ", testClient.topicID);
    }

    //Perform a disconnect with the server.
    returnCode = disconnect(&testClient, 0);

    if(returnCode != Q_NO_ERR){
        puts("Error with disconnect");
    }

    transport_close();
    return returnCode;
}


