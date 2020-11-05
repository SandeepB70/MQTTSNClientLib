//Test for the subscribe message.

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "MQTTSNPublish.h"
#include "transport.h"
#include "Connect.h"
#include "Disconnect.h"
#include "Publish.h"
#include "Register.h"
#include "Subscribe.h"
#include "RegAck.h"
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
    testClient.clientID = "WildcardSub";

    returnCode = connect(&testClient, keepAlive, willFlag, clnSession);

    //Check the return code.
    returnCodeHandler(returnCode);

    uint16_t msgID = 1;

    //Will be used to set the appropriate flags for the Subscribe message
    MQTTSNFlags flags;

    //First initialize all the flags to off.
    flags.all = 0;

    //Indicate a normal topic name
    flags.bits.topicIdType = 0b00;
    
    //The topic name the client will subscribe to
    MQTTSN_topicid topic; 

    topic.type = MQTTSN_TOPIC_TYPE_NORMAL;
    //Name of the topic to subscribe to
    topic.data.long_.name = "House1/#";

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

    printf("%s%s\n", "Subscribed to: ", topic.data.long_.name);

    //Wait for a register message, which indicates the server has a publish message for the client.
    returnCode = readReg(&testClient, &msgID);

    returnCodeHandler(returnCode);

    if(returnCode != Q_NO_ERR){
        return 1;
    }

    //The return code which signifies "accepted" for the RegAck message.
    uint8_t msgReturnCode = 0;
    //Send out a RegAck to acknowledge recept of the register message and 
    //the server can begin sending publish messages.
    returnCode = regAck(&testClient, testClient.topicID, msgID, msgReturnCode);

    returnCodeHandler(returnCode);

    if(returnCode != Q_NO_ERR){
        return 1;
    }


    returnCode = readPub(testClient.topicID);
    //Checking to make sure the message could be read.
    if(returnCode != Q_NO_ERR){
        if(returnCode == Q_ERR_SubTopic){
            puts("Mismatching topic IDs");
            return 1;
        }
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


