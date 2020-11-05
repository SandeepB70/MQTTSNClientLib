//Test for the publish message.

#include <stdlib.h>


#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "Disconnect.h"
#include "Publish.h"
#include "Register.h"
#include "Util.h"
#include "ErrorCodes.h"

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
    testClient.clientID = "ClientPub1";

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
        return 1;
    }

    //Represents the message ID
    uint16_t msgID = 1;

    returnCode = reg(&testClient, msgID, &topicHolder, &(testClient.topicID));

    //Check the return code.
    returnCodeHandler(returnCode);

    if(returnCode == Q_NO_ERR){
        printf("%s%d\n", "TopicID: ", testClient.topicID);
    }
    else{
        puts("Error with register");
        return 1;
    }

    //Publish to the same topicID received by the server.

    //Increment the msgID so there is no confusion for which message is being referenced
    ++msgID;

    //Will be used to set the appropriate flags for the Publish message
    MQTTSNFlags flags;

    //First initialize all the flags to off.
    flags.all = 0;

    //Indicate a normal topicID
    flags.bits.topicIdType = 0b00;
    flags.bits.retain = 0b0;
    
    //The data to be sent with the publish message
    unsigned char* msgData = (unsigned char*)"Hi there from ClientPub1.";

    returnCode = publish(&testClient, &flags, testClient.topicID, msgID, msgData);

    //Check the return code.
    returnCodeHandler(returnCode);

    if(returnCode != Q_NO_ERR)
    {
        puts("Publish error");
        return 1;
    }

    ++msgID;
    //The data to be sent with the publish message
    unsigned char* msgData2 = (unsigned char*)"Second hello message.";

    returnCode = publish(&testClient, &flags, testClient.topicID, msgID, msgData2);

    //Check the return code.
    returnCodeHandler(returnCode);

    if(returnCode != Q_NO_ERR)
    {
        puts("Publish error");
        return 1;
    }

    //Perform a disconnect with the server.
    returnCode = disconnect(&testClient, 0);

    if(returnCode != Q_NO_ERR){
        puts("Error with disconnect");
        return 1;
    }

    transport_close();
    return 0;
}


