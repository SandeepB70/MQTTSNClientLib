//Test for the publish message.

#include <stdlib.h>
#include <string.h>


#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "Disconnect.h"
#include "Publish.h"
#include "Subscribe.h"
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
    testClient.clientID = "ClientPubSub1";
    //The topic ID this Client will publish to.
    uint16_t pubTopicID = 0;

    returnCode = connect(&testClient, keepAlive, willFlag, clnSession);

    //Check the return code.
    returnCodeHandler(returnCode);

    //The struct that will hold the topic name.
    MQTTSNString topicHolder;

    //The topic name to register.
    char *topicName = "TestingTopic1";

    returnCode = MQTTSNStrCreate(&topicHolder, topicName);

    if(returnCode != Q_NO_ERR){
        puts("Problem with String create");
        return 1;
    }

    //Represents the message ID
    uint16_t msgID = 1;

    //Register the topic.
    returnCode = reg(&testClient, msgID, &topicHolder, &(pubTopicID));

    //Check the return code.
    returnCodeHandler(returnCode);

    if(returnCode == Q_NO_ERR){
        printf("%s%d\n", "TopicID: ", pubTopicID);
    }
    else{
        puts("Error with register");
        return 1;
    }

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
    unsigned char* msgData = (unsigned char*)"Hi there from ClientPubSub1.";

    //Publish to the same topicID received by the server.
    returnCode = publish(&testClient, &flags, pubTopicID, msgID, msgData);

    //Check the return code.
    returnCodeHandler(returnCode);

    if(returnCode != Q_NO_ERR)
    {
        puts("Publish error");
        return 1;
    }

    //Increment the msg ID to avoid confusion about which message this is.
    ++msgID;

    //First initialize all the flags to off.
    flags.all = 0;

    //Indicate a normal topic name
    flags.bits.topicIdType = 0b00;
    
    //The topic name the client will subscribe to
    MQTTSN_topicid topic; 

    topic.type = MQTTSN_TOPIC_TYPE_NORMAL;
    //Name of the topic to subscribe to
    topic.data.long_.name = "TestingTopic2";

    //Obtain the length of the topic name and assign it to the len member of topic
    int topicLen = (int)strlen(topic.data.long_.name);
    if(topicLen > 0){
        topic.data.long_.len = (size_t)topicLen;
    }

    //Subscribe to a topic name another client will be publishing to.
    returnCode = subscribe(&testClient, &topic, flags, msgID);

    //Check the return code.
    returnCodeHandler(returnCode);

    if(returnCode != Q_NO_ERR)
    {
        puts("Subscribe error");
        return 1;
    }

    //DEBUG
    printf("%s%d\n", "Subscribed to: ", testClient.topicID);

    returnCode = readPub(testClient.topicID);

    //Perform a disconnect with the server.
    returnCode = disconnect(&testClient, 0);

    if(returnCode != Q_NO_ERR){
        puts("Error with disconnect");
        return 1;
    }

    transport_close();
    return 0;
}


