/**
 * The event handler for the MQTT-SN Client
 * 
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "Events.h"
#include "Util.h"
#include "ErrorCodes.h"
#include "transport.h"
#include "Connect.h"
#include "WillTopic.h"
#include "WillMsg.h"
#include "Register.h"
#include "RegAck.h"
#include "PubAck.h"
#include "Disconnect.h"
#include "PubRecRelComp.h"
#include "PingResp.h"
#include "PingReq.h"
#include "Publish.h"
#include "StackTrace.h"

int event_handler(Client_Event_t *event); //prototype

int event_handler(Client_Event_t *event)
{

    //All these variables should be defined outside the loop.
    bool loopFlag = true;
    int returnCode = Q_ERR_Unknown;

    //Needed to increment the send_msgID values since it is a uint16_t
    //uint8_t add1 = 1;

    //The maximum number of times certain message will be attempted to be retransmitted.
    uint8_t maxRetry = 3;
    //The current number of times a message has been retransmitted out.
    uint8_t retries = 0;
    //Used to represent flag variables for any messages that might need them.
    MQTTSNFlags flags;
    MQTTSNString clientString;

    //How often a pingreq message should be sent in seconds.
    int ping_req_timeout = event->duration - 5;
    //How long the client is asleep for in seconds.
    int sleep_timeout = 15;
    //How often the client will publish.
    int publish_timeout = 3;

    //Indicates if the client will be going back to sleep.
    bool sleepFlag = false;
    //Indicates if the client will need to register any topics
    bool registerFlag = true;
    //Indicates if the client will send any publish messages.
    bool publishFlag = false;
    bool pub0 = true;
    bool pub1 = false;
    bool pub2 = false;

    //Used to keep track of how much time has past since the last pingreq.
    //Starts when the client has connected.
    time_t timer_PingReq = time(NULL);
    //Starts once the client goes to sleep.
    time_t timer_Sleep = (time_t) -1;
    //timer for publishing.
    time_t timer_Publish = time(NULL);
    

while(loopFlag)
{
    flags.all = 0;

    switch(event->eventID)
    {
        case Q_CONNECTING:
            //Check if a message was received from the server.
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode == Q_MsgPending) {
                //Read the message
                returnCode = readMsg(event);
                //If it is a Connack message, move into the Q_CONNECTED state.
                if(returnCode == Q_ConnackRead) {
                    puts("Client connected.");
                    retries = 0;
                    event->eventID = Q_CONNECTED;
                    timer_PingReq = time(NULL);
                    break;
                //If it is a WillTopicReq message, send a WillTopic message and 
                //move into the Q_WILL_TOP_REQ state.
                } else if(returnCode == Q_WillTopReq) {
                    //Need to send a WillTopic message.
                    //Change the # symbol
                    char *will_top = "Client_#/Will";
                    if(MQTTSNStrCreate(&clientString, will_top) != Q_NO_ERR) {
                        puts("Error with string creation for WillTopic.");
                        disconnect(event->client, 0);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    //Send a WillTopic message.
                    returnCode = willTopic(event->client, flags, clientString);
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with sending WillTopic message.");
                        returnCodeHandler(returnCode);
                        disconnect(event->client, 0);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    event->eventID = Q_WILL_TOP_REQ;
                    break;
                } else {
                    puts("Unexpected error/message in CONNECTING State.");
                    printf("ReturnCode: %d\n", returnCode);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
            //Something might be wrong with connection settings.
            } else if (returnCode == Q_NoMsg) {
                puts("Attempting to connect to server...");
                if(retries < maxRetry){
                    event->eventID = Q_CONNECTING;
                    ++retries;
                    break;
                }else {
                    puts("No message received, check connection settings.");
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
                
            //Unknown error.
            } else {
                puts("Error with readMsg in CONNECTING State.");
                event->eventID = Q_DISCONNECTED;
                break;
            }

        case Q_CONNECTED:
            /************************************************/
            /************************************************/
            //ANY messages to be sent should be placed in this area.
            /************************************************/
            /************************************************/
            if(registerFlag){
                char *topicname = "PubClientV1/Test";
                returnCode = MQTTSNStrCreate(&clientString, topicname);
                returnCode = reg(event->client, event->send_msgID, &clientString);
                if(returnCode != Q_NO_ERR) {
                    puts("Error with topic registering");
                    disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
                event->eventID = Q_REGISTERING;
                break;
            }
        
            if(publishFlag){
                if(time(NULL) - timer_Publish > publish_timeout){
                    //Send Publish message at qos0
                    if(pub0) {
                        //Used to get the message id.
                        char num[10];
                        //The data that will be sent out in the publish message.
                        char pub[100] = "Qos 0, msgID: ";
                        sprintf(num, "%hu", event->send_msgID);
                        strcat(pub, num);
                        //unsigned char *data = (unsigned char*)pub;
                        flags.bits.QoS = 0b00;
                        printf("Publishing with msgID: %hu\n", event->send_msgID);
                        returnCode = publish(event->client, &flags, event->client->pub_topicID[0], event->send_msgID, (unsigned char*)pub);
                        if(returnCode != Q_NO_ERR){
                            puts("Errror with publishing qos0");
                            disconnect(event->client, 0);
                            event->eventID = Q_DISCONNECTED;
                            break;
                        }
                        event->send_msgID = (uint16_t)(event->send_msgID + 1);
                        //Reset the timer.
                        timer_Publish = time(NULL);
                        //Next published message will have qos 1.
                        pub0 = false;
                        pub1 = true;
                        break;
                    //Send Publish message at qos1
                    } else if(pub1) {
                        //Used to get the message id.
                        char num[10];
                        //The data that will be sent out in the publish message.
                        char pub[100] = "Qos 1, msgID: ";
                        sprintf(num, "%hu", event->send_msgID);
                        strcat(pub, num);
                        printf("%s\n", pub);
                        //unsigned char *data = (unsigned char*)pub;
                        flags.bits.QoS = 0b01;
                        printf("Publishing with msgID: %hu\n", event->send_msgID);
                        returnCode = publish(event->client, &flags, event->client->pub_topicID[0], event->send_msgID, (unsigned char*)pub);
                        if(returnCode != Q_NO_ERR){
                            puts("Errror with publishing qos1");
                            disconnect(event->client, 0);
                            event->eventID = Q_DISCONNECTED;
                            break;
                        }
                        //Next published message will have qos 2.
                        pub1 = false;
                        pub2 = true;
                        event->eventID = Q_PUBLISH;
                        break;
                    }
                    else if (pub2) {
                        //Used to get the message id.
                        char num[10];
                        //The data that will be sent out in the publish message.
                        char pub[100] = "Qos 2, msgID: ";
                        sprintf(num, "%hu", event->send_msgID);
                        strcat(pub, num);
                        //unsigned char *data = (unsigned char*)pub;
                        flags.bits.QoS = 0b10;
                        printf("Publishing with msgID: %hu\n", event->send_msgID);
                        returnCode = publish(event->client, &flags, event->client->pub_topicID[0], event->send_msgID, (unsigned char*)pub);
                        if(returnCode != Q_NO_ERR){
                            puts("Errror with publishing qos2");
                            disconnect(event->client, 0);
                            event->eventID = Q_DISCONNECTED;
                            break;
                        }
                        //Next published message will have qos 0.
                        pub2 = false;
                        pub0 = true;
                        event->eventID = Q_PUBLISH;
                        break;
                    }
                }
            }//End if(publish)
            //First check if a pingreq needs to be sent.
            if(time(NULL) - timer_PingReq > ping_req_timeout) {
                //Create an MQTTSNString containing the an empty string so the clientID is not sent over.
                MQTTSNStrCreate(&clientString, NULL);
                //Send the pingReq.
                returnCode = pingReq(event->client, &clientString);
                event->eventID = Q_CLIENT_PING;
                break;
            }
            //Checks if a sleeping client needs to wake up and check for messages.
            if(sleepFlag){
                event->eventID = Q_SLEEP;
                break;
            }
            //Check if there are any messages to be read.
            if(msgReceived(event->client->mySocket) != Q_MsgPending) {
                break;
            }
            //Read the message and then check what type it is.
            returnCode = readMsg(event);
            //Have to send a PingResp message back to server.
            if(returnCode == Q_PingReqRead) {
                event->eventID = Q_SERVER_PING;
                break;
            }
            //publish with qos 0 received. Just loop back to connected state.
            if(returnCode == Q_pubQos0) {
                break;
            }
            //publish with qos 1 received, need to send a pubAck
            if(returnCode == Q_pubQos1) {
                event->eventID = Q_RCV_QOS1;
                break;
            }
            //publish with qos 2 received, need to exchange ack messages with server.
            if(returnCode == Q_pubQos2) {
                //Send a pubRec message.
                returnCode = pubRecRelComp(event->client, event->msgID, MQTTSN_PUBREC);
                if(returnCode != Q_NO_ERR){
                    puts("Error with sending pubRec.");
                    break;
                }
                event->eventID = Q_RCV_QOS2;
                break;
            }
            //Need to send a PubAck with a rejection return code.
            if(returnCode == Q_ERR_WrongTopicID) {
                returnCode = pubAck(event->client, event->topicID, event->msgID, MQTTSN_RC_REJECTED_INVALID_TOPIC_ID);
                if(returnCode != Q_NO_ERR){
                    puts("Sending of PubAck rejection message failed.");
                    break;
                }
                break;
            }
            //Register message received so a RegAck needs to be sent back with an accepted return code.
            //Then it can begin receiving any publish messages.
            if(returnCode == Q_Subscribed || returnCode == Q_Wildcard) {
                returnCode = regAck(event->client, event->topicID, event->msgID, MQTTSN_RC_ACCEPTED);
                if(returnCode != Q_NO_ERR) {
                    puts("Sending of RegAck accept failed.");
                    returnCode = disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
                break;
            }
            //Register message received with invalid topicID so send a RegAck with a rejection return code.
            if(returnCode == Q_RejectReg){
                returnCode = regAck(event->client, event->topicID, event->msgID, MQTTSN_RC_REJECTED_INVALID_TOPIC_ID);
                if(returnCode != Q_NO_ERR) {
                    puts("Sending of RegAck reject failed.");
                    returnCode = disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
                break;
            }

            break; //Last break for Q_CONNECTED

        case Q_RCV_QOS1:
            returnCode = pubAck(event->client, event->topicID, event->msgID, MQTTSN_RC_ACCEPTED);
            if(returnCode != Q_NO_ERR){
                puts("Failed to send pubAck qos1.");
                event->eventID = Q_CONNECTED;
                break;
            }
            break;

        case Q_RCV_QOS2:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending){
                puts("Error: expecting PubRel from server.");
                event->eventID = Q_CONNECTED;
                break;
            }
            //Read the message and check if it is what is expected.
            returnCode = readMsg(event);
            if(returnCode != Q_PubRelRead){
                puts("Unexpected error/message in Q_WILL_TOP_REQ");
                event->eventID = Q_CONNECTED;
                break;
            }
            //Need to send out a PubComp
            returnCode = pubRecRelComp(event->client, event->msgID, MQTTSN_PUBCOMP);
            if(returnCode != Q_NO_ERR){
                puts("Error with sending pubComp");
                break;
            }
            break;

        case Q_SLEEP:
            if(time(NULL) - timer_Sleep < sleep_timeout - 5){
                break;
            }
            //Send a pingReq message to wake the client from sleep.
            MQTTSNStrCreate(&clientString, event->client->clientID);
            returnCode = pingReq(event->client, &clientString);
            if(returnCode != Q_NO_ERR){
                puts("Sleeping client failed to send PingReq");
                event->eventID = Q_CONNECTED;
                break;
            }
            event->eventID = Q_CONNECTED;
            break;
        
        case Q_PUBLISH:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("Error: No message received for publish qos.");
                event->eventID = Q_CONNECTED;
                //Reset the timer.
                timer_Publish = time(NULL);
                break;
            }
            returnCode = readMsg(event);
            if(returnCode == Q_PubAckRead){
                event->send_msgID = (uint16_t)(event->send_msgID + 1);
                puts("PubAck received.");
                //Reset the timer.
                timer_Publish = time(NULL);
                event->eventID = Q_CONNECTED;
                break;
            } else if (returnCode == Q_PubRecRead){
                puts("PubRec received.");
                returnCode = pubRecRelComp(event->client, event->msgID, MQTTSN_PUBREL);
                if(returnCode != Q_NO_ERR){
                    puts("Error: Failed to send PubRel");
                    //Reset the timer.
                    timer_Publish = time(NULL);
                    break;
                }
                event->eventID = Q_PUB_QOS2;
                break;
            } else {
                puts("Error with publish qos");
                //Reset the timer.
                timer_Publish = time(NULL);
                event->eventID = Q_CONNECTED;
                break;
            }
            break;

        case Q_PUB_QOS2:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("Error: No message for publish qos2");
                //Reset the timer.
                timer_Publish = time(NULL);
                event->eventID = Q_CONNECTED;
                break;
            }
            returnCode = readMsg(event);
            if(returnCode != Q_PubCompRead){
                puts("Error with receiving pubComp");
                //Reset the timer.
                timer_Publish = time(NULL);
                event->eventID = Q_CONNECTED;
                break;
            }
            puts("Received PubComp");
            event->send_msgID = (uint16_t)(event->send_msgID + 1);
            //Reset the timer.
            timer_Publish = time(NULL);
            event->eventID = Q_CONNECTED;
            break;
        //Should transition to this state after receiving WillTopicReq message when Connecting.
        case Q_WILL_TOP_REQ:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode == Q_MsgPending) {
                returnCode = readMsg(event);
                //Check if a WillMsgReq has been received.
                if(returnCode == Q_WillMsgReq) {
                    //Change the # symbol.
                    char *msg = "Client_# down.";
                    if (MQTTSNStrCreate(&clientString, msg) != Q_NO_ERR) {
                        puts("Error with string creation for WillMsg.");
                        disconnect(event->client, 0);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    //Send a WillMsg
                    returnCode = willMsg(event->client, &clientString);
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with sending WillMsg message.");
                        returnCodeHandler(returnCode);
                        disconnect(event->client, 0);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    //Transition to the Q_WILL_MSG_REQ state.
                    event->eventID = Q_WILL_MSG_REQ;
                    break;
                } else {
                    puts("Unexpected error/message in Q_WILL_TOP_REQ");
                    disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
            } else {
                puts("No message received.");
                disconnect(event->client, 0);
                event->eventID = Q_DISCONNECTED;
                break;
            }

        case Q_WILL_MSG_REQ:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode == Q_MsgPending) {
                returnCode = readMsg(event);
                if(returnCode != Q_ConnackRead){
                    puts("Unexpected error/message in Q_WILL_MSG_REQ");
                    //break the loop, indicate couldn't connect to server.
                    disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
                //WillMsg has been acknowledged so client is now connected.
                event->eventID = Q_CONNECTED;
                //Start the PingReq timer
                timer_PingReq = time(NULL);
                break;
            } else {
                puts("No message received.");
                disconnect(event->client, 0);
                event->eventID = Q_DISCONNECTED;
                break;
            }

        case Q_WILL_MSG_UPD:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("No message received.");
                //Transition back to the CONNECTED state.
                event->eventID = Q_CONNECTED;
                break;
            }
            //Otherwise read in the message.
            returnCode = readMsg(event);
            //Check if this is the expected message.
            if(returnCode != Q_MsgRespRead){
                puts("Error in Q_WILL_MSG_UPD");
                printf("Return Code: %d\n", returnCode);
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
                break;
            }
            event->eventID = Q_CONNECTED;
            break;

        case Q_WILL_TOP_UPD:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("No message received.");
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
                break;
            }
            //Otherwise read in the message.
            returnCode = readMsg(event);
            //Check if this is the expected message.
            if(returnCode != Q_TopicRespRead) {
                puts("Error in Q_WILL_MSG_UPD");
                printf("Return Code: %d\n", returnCode);
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
            }
            event->eventID = Q_CONNECTED;
            break;

        case Q_REGISTERING:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("No message received.");
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
                break;
            }
            //Otherwise read in the message.
            returnCode = readMsg(event);
            //Check if this is the expected message.
            if(returnCode != Q_RegAckRead) {
                puts("Error in Q_REGISTERING");
                printf("Return Code: %d\n", returnCode);
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
            }
            registerFlag = false;
            event->eventID = Q_CONNECTED;
            //Allow the client to start sending publish messages
            publishFlag = true;
            event->send_msgID = (uint16_t)(event->send_msgID + 1);
            break;

        case Q_SUBSCRIBING:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("No message received.");
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
                break;
            }
            //Otherwise read in the message.
            returnCode = readMsg(event);
            //Check if this is the expected message.
            if(returnCode != Q_SubAckRead) {
                puts("Error in Q_SUBSCRIBING");
                printf("Return Code: %d\n", returnCode);
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
            }
            event->eventID = Q_CONNECTED;
            break;

        case Q_DISCONNECTING:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("No message received.");
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
                break;
            }
            //Otherwise read in the message.
            returnCode = readMsg(event);
            //Check if this is the expected message.
            if(returnCode != Q_DisconRead) {
                puts("Error in Q_DISCONNECTING");
                printf("Return Code: %d\n", returnCode);
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
                break;
            }
            returnCode = Q_NO_ERR;
            event->eventID = Q_DISCONNECTED;
            break;

        case Q_DISCONNECTED:
            transport_close();
            loopFlag = false;
            break;

        case Q_CLIENT_PING:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                //Retry sending a ping to the server.
                if(retries <= maxRetry){
                    puts(" No ping received. Retrying PingReq");
                    //Create an MQTTSNString containing the an empty string so the clientID is not sent over.
                    MQTTSNStrCreate(&clientString, NULL);
                    //Send the pingReq.
                    returnCode = pingReq(event->client, &clientString);
                    //If unsuccessful, shutdown the client by going to disconnected state.
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with ping transmission.");
                        disconnect(event->client, 0);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    ++retries;
                    //Transition back to the Q_CLIENT_PING state to check if a pingresp came.
                    event->eventID = Q_CLIENT_PING;
                    break;
                //Close off the socket
                } else {
                    puts("Max ping retries reached.");
                    //disconnect(event->client, 0);
                    timer_PingReq = time(NULL);
                    event->eventID = Q_CONNECTED;
                    //returnCode = Q_ERR_NoPingResp;
                    break;
                }
            }
            returnCode = readMsg(event);
            if(returnCode != Q_PingRespRead){
                puts("Error in Q_CLIENT_PING");
                printf("Return Code: %d\n", returnCode);
                //Transition back to the connected state.
                event->eventID = Q_CONNECTED;
                break;
            }
            //Reset the number of retries and timer.
            retries = 0;
            timer_PingReq = time(NULL);
            break;
        
        case Q_SERVER_PING:
            //Need to send out a PingResp
            returnCode = pingResp(event->client);
            //If pingResp failed to send, just transition to the Connected state
            if(returnCode != Q_NO_ERR) {
                puts("pingResp failed to send.");
                event->eventID = Q_CONNECTED;
                break;
            }
            event->eventID = Q_CONNECTED;
            break;
            
        case Q_UNSUBSCRIBE:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending){
                puts("No message received for Q_UNSUBSCRIBE");
                event->eventID = Q_CONNECTED;
                break;
            }
            //Otherwise read in the message and check if it is Unsuback.
            returnCode = readMsg(event);
            if(returnCode != Q_UnsubackRead){
                puts("Error in Q_SUBSCRIBE");
                event->eventID = Q_CONNECTED;
                break;
            }
            event->eventID = Q_CONNECTED;
            break;
    }//End switch

}// End while loop
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//end event_handler



int main(void)
{
    int returnCode;
    //The duration portion for the connect message.
    uint16_t keepAlive = 20;
    //Keep the willflag off since we are just testing the connect message only.
    uint8_t willFlag = 0;
    //The Clean Session flag, which will be set to on.
    uint8_t clnSession = 1;
    Client_t testClient;
    testClient.destinationPort = 10000;
    testClient.host = "10.0.2.15";
    testClient.clientID = "ClientPub1";
    testClient.subscribe_Num = 0;
    testClient.publish_Num = 0;
    testClient.wildcard_Sub = false;
    testClient.sub_Wild_Num = 0;
    returnCode = connect(&testClient, keepAlive, willFlag, clnSession);

    if(returnCode != Q_NO_ERR){
        puts("Could not connect.");
        return 1;
    }

    Client_Event_t event;
    event.client = &testClient;
    event.duration = keepAlive;
    event.eventID = Q_CONNECTING;
    event.send_msgID = 1;

    printf("Client Socket: %d\n", event.client->mySocket);

    returnCode = event_handler(&event);
    returnCodeHandler(returnCode);
    return 0;
}

