//Test for the disconnect message.

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "Subscribe.h"
#include "Disconnect.h"
#include "PingReq.h"
#include "ErrorCodes.h"
#include "Util.h"

int main(void)
{

    int returnCode;
    //Used for the duration portion of the Connect message
    uint16_t keepAlive = 15;
    //Will be used to set the duration for the disconnect message.
    uint16_t duration = 17;
    //Sets the will flag in the connect message.
    uint8_t willF = 0;
    //Set the clean session flag for the connect message.
    uint8_t clnSession = 1;

    Client_t testClient;
    testClient.destinationPort = 10000;
    testClient.host = "10.0.2.15";
    testClient.clientID = "SleepingClient";
    bool sleep = true;
    //Used to time when the client should wake up.
    long int begin, end;
    
    //Connect to the server.
    returnCode = connect(&testClient, keepAlive, willF, clnSession);
    //Check the return code to make sure the connection was successful.
    returnCodeHandler(returnCode);

    if(returnCode != Q_NO_ERR){
        return 1;
    }

    //Subscribe to a topic so messages will be buffered for the client while it sleeps.
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
    topic.data.long_.name = "TestingTopic1";

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

    //Send a disconnect message indicating the client will be going to sleep.
    returnCode = disconnect(&testClient, duration);
    //Check the return code to make sure the disconnect was successful.
    returnCodeHandler(returnCode);

    //Start the timer for long the timer should be sleeping.
    begin = time(NULL);
    
    //Make sure the server has acknowledged the disconnect.
    if(returnCode != Q_NO_ERR){
        return 1;
    }

    printf("\n%s%d%s\n", "Sleeping for: ", duration, " seconds.");

    //Loop until the client has slept for 10 seconds.
    while(sleep){
        end = time(NULL);
        if(end - begin > 10){
            sleep = false;
            puts("Waking up");
        }
    }

    //Need this to pass the client ID to the PingReq message.
    MQTTSNString clientID;
    MQTTSNStrCreate(&clientID, testClient.clientID);

    //Send a PingReq message.
    returnCode = pingReq(&testClient, &clientID);
    //Check the return code of the PingReq message.
    returnCodeHandler(returnCode);

    //Perform a disconnect with the server.
    returnCode = disconnect(&testClient, 0);

    if(returnCode != Q_NO_ERR){
        puts("Error with disconnect");
        return 1;
    }

	transport_close();

    return returnCode;
}


