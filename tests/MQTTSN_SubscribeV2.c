/**
 * Percentage Contribution: Sandeep Bindra (60%), Bianca Lavaud (20%), Amanda Lai (20%)
 * Subscribe only MQTT-SN Client
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
#include "Subscribe.h"
#include "StackTrace.h"

int client_machine(Client_Event_t *event); //prototype

/**
 * Represents a client that only subscribes to one topic. 
 * @param event Contains a Client_t struct and the necessary information 
 * for when exchanging acknowledgement messages from the server. 
 * @return An int, will not be checked.
 */ 
int client_machine(Client_Event_t *event)
{
    //Allows the client to keep looping over the switch statement, only set to false
    //when the client goes to Q_Disconnected state.
    bool loopFlag = true;
    //Used to check the return codes when sending out messages and receiving messages.
    int returnCode = Q_ERR_Unknown;

    //The maximum number of times certain message will be attempted to be retransmitted.
    uint8_t maxRetry = 3;
    //The current number of times a message has been retransmitted out.
    uint8_t retries = 0;
    //Used to represent flag variables for any messages that might need them.
    MQTTSNFlags flags;
    //This struct is needed for certain portions of some messages by
    //the PAHO library.
    MQTTSNString clientString;

    //How often a pingreq message should be sent in seconds.
    int ping_req_timeout = event->duration - 5;
    //How long the client is asleep for in seconds.
    int sleep_timeout = 15;

    //Indicates if the client will be going back to sleep.
    bool sleepFlag = false;
    //Indicates if the client will need to subscribe to any topics.
    bool subscribeFlag = true;
    bool sub0 = true;
    //bool sub1 = false;
    //bool sub2 = false;

    //Used to keep track of how much time has past since the last pingreq.
    time_t timer_PingReq = time(NULL);
    //Starts once the client goes to sleep.
    time_t timer_Sleep = (time_t) -1;
    

while(loopFlag)
{
    //Reset all flags to 0
    flags.all = 0;
    //The current time. This is used to measure how much time has passed for the timers and reset them.
    time_t now = time(NULL);
    switch(event->eventID)
    {
        //Initial Stage for the client to start connecting after sending out a connect message. 
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
                    //Set the ping timer
                    timer_PingReq = now;
                    break;
                //If it is a WillTopicReq message, send a WillTopic message and 
                //move into the Q_WILL_TOP_REQ state.
                } else if(returnCode == Q_WillTopReq) {
                    //Need to send a WillTopic message.
                    //A placeholder, can be changed to whatever the desired name is for the Will Topic
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
            break; //End for Q_CONNECTING break
        
        //The "home" state a client constantly loops back to. 
        //This is where the client will subscribe, publish, and send out any other messages
        //unique to this client. 
        case Q_CONNECTED:
            /************************************************/
            /************************************************/
            //ANY messages to be sent should be placed in this area.
            /************************************************/
            /************************************************/
            if(subscribeFlag){
                if(sub0) {
                    //Indicate a normal topic name
                    flags.bits.topicIdType = 0b00;
    
                    //The topic name the client will subscribe to
                    MQTTSN_topicid topic; 

                    topic.type = MQTTSN_TOPIC_TYPE_NORMAL;
                    //Name of the topic to subscribe to
                    topic.data.long_.name = "PubClientV2/Test2";

                    //Obtain the length of the topic name and assign it to the len member of MQTTSN_topicid 
                    int topicLen = (int)strlen(topic.data.long_.name);
                    if(topicLen > 0) {
                    topic.data.long_.len = (size_t)topicLen;
                    }
                    //Subscribe to QoS level 2 (this is irrelevant, however, 
                    //because the publisher dictates the QoS level with the Gateway being used)
                    flags.bits.QoS = 0b10;
                    //Send out the subscribe message and check if it was successful.
                    returnCode = subscribe(event->client, &topic, flags, event->send_msgID);
                    if(returnCode != Q_NO_ERR){
                        puts("Error with subscribing.");
                        returnCodeHandler(returnCode);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    event->qos = flags.bits.QoS;
                    //Make these variables false to ensure the client does not try subscribing again.
                    sub0 = false;
                    subscribeFlag = false;
                    event->eventID = Q_SUBSCRIBING;
                    break;
                }
                break;
            } //End if(subscribeFlag)

            //First check if a pingreq needs to be sent.
            if(now - timer_PingReq > ping_req_timeout) {
                puts("Pinging"); //DEBUG
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
                if(returnCode != Q_NO_ERR) {
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
            break; //End break for Q_CONNECTED

        //Client received a publish message with QoS level 1 so it will send back a pubAck message.
        case Q_RCV_QOS1:
            returnCode = pubAck(event->client, event->topicID, event->msgID, MQTTSN_RC_ACCEPTED);
            if(returnCode != Q_NO_ERR){
                puts("Failed to send pubAck qos1.");
                event->eventID = Q_CONNECTED;
                break;
            }
            event->eventID = Q_CONNECTED;
            break; //End break for Q_RCV_QOS1

        //Client received a publish message with QoS level 2 and has already sent out a PubRec.
        //Client is expecting a PubRel and will then send a PubComp. 
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
                puts("Unexpected error/message in Q_RCV_QOS2");
                event->eventID = Q_CONNECTED;
                break;
            }
            //Need to send out a PubComp
            returnCode = pubRecRelComp(event->client, event->msgID, MQTTSN_PUBCOMP);
            if(returnCode != Q_NO_ERR){
                puts("Error with sending pubComp");
                break;
            }
            event->eventID = Q_CONNECTED;
            break; //End break for Q_RCV_QOS2

        //Used when the client is sleeping
        case Q_SLEEP:
            if(now - timer_Sleep < sleep_timeout - 5){
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
            break; //End break for Q_SLEEP
        
        //Entered when client sends a Publish message with Qos level 1 or 2.
        case Q_PUBLISH:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("Error: No message received for publish qos.");
                event->eventID = Q_CONNECTED;
                //Reset the timer.
                //timer_Publish = now;
                break;
            }
            returnCode = readMsg(event);
            //Check the acknowledgement message received. If it is a pubAck, this was Qos level 1 and if
            //it is a pubRel, this was Qos level 2. 
            if(returnCode == Q_PubAckRead){
                //Increment the msgID for the next message that gets sent out.
                event->send_msgID = (uint16_t)(event->send_msgID + 1);
                puts("PubAck received.");
                //Reset the timer.
                //timer_Publish = now;
                event->eventID = Q_CONNECTED;
                break;
            } else if (returnCode == Q_PubRecRead){
                puts("PubRec received.");
                returnCode = pubRecRelComp(event->client, event->send_msgID, MQTTSN_PUBREL);
                if(returnCode != Q_NO_ERR){
                    puts("Error: Failed to send PubRel");
                    //Reset the timer, only used if publishing.
                    //timer_Publish = now;
                    break;
                }
                event->eventID = Q_PUB_QOS2;
                break;
            } else {
                puts("Error with publish qos");
                //Reset the timer.
                //timer_Publish = now;
                event->eventID = Q_CONNECTED;
                break;
            }
            break; //End break for Q_PUBLISH

        //Used when the client has sent out a publish message with Qos level 2.
        case Q_PUB_QOS2:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("Error: No message for publish qos2");
                //Reset the timer.
                //timer_Publish = now;
                event->eventID = Q_CONNECTED;
                break;
            }
            returnCode = readMsg(event);
            if(returnCode != Q_PubCompRead){
                puts("Error with receiving pubComp");
                //Reset the timer.
                //timer_Publish = now;
                event->eventID = Q_CONNECTED;
                break;
            }
            puts("Received PubComp");
            //Increment the msgID for the next message that gets sent out.
            event->send_msgID = (uint16_t)(event->send_msgID + 1);
            //Reset the timer.
            //timer_Publish = now;
            event->eventID = Q_CONNECTED;
            break; //End break for Q_PUB_QOS2

        //Should transition to this state after receiving WillTopicReq message when Connecting.
        case Q_WILL_TOP_REQ:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode == Q_MsgPending) {
                returnCode = readMsg(event);
                //Check if a WillMsgReq has been received.
                if(returnCode == Q_WillMsgReq) {
                    //A placeholder, can be changed to whatever the desired name is for the Will Topic
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
            break; //End break for Q_WILL_TOP_REQ

        //Last stage to enter when Client connects with Will Flag turned on.
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
                timer_PingReq = now;
                break;
            } else {
                puts("No message received.");
                disconnect(event->client, 0);
                event->eventID = Q_DISCONNECTED;
                break;
            }
            break; //End break for Q_WILL_MSG_REQ

        //Used when the Client sends a WillMsgUpd, expecting a WillMsgResp from the Gateway
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
            break; //End break for Q_WILL_MSG_UPD

        //Used when the Client sends a WillTopicUpd, expecting a WillTopicResp from the Gateway
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
            break; //End break for Q_WILL_TOP_UPD

        //Used when the client is trying to register a topic name with the Gateway for publishing.
        //Expecting a RegAck with an accepted return code since a register message has already been sent.
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
            //Only used for a publishing client
            //registerFlag = false;
            event->eventID = Q_CONNECTED;
            //Only used for a publishing client
            //Allow the client to start sending publish messages
            //publishFlag = true;
            //Increment the message ID if this client has to send out any more messages.
            event->send_msgID = (uint16_t)(event->send_msgID + 1);
            break; //End break for Q_REGISTERING

        //Used when subscribing to a topic.
        case Q_SUBSCRIBING:
            //Expecting a SubAck message with a returncode of Accepted.
            returnCode = msgReceived(event->client->mySocket);
            //Check if this is the expected message.
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
            //Increment the message ID if this client has to send out any more messages.
            event->send_msgID = (uint16_t)(event->send_msgID + 1);
            event->eventID = Q_CONNECTED;
            break; //End break for Q_SUBSCRIBING

        //Used when the client sends a disconnect message and is awaiting a disconnect message, either
        //for a clean disconnect or for the client to enter into sleep.
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
            break; //End break for Q_DISCONNECTING

        //Stops the loop and closes the client socket. Used when the client cleanly disconnects or as an "emergency stop"
        //for the client.
        case Q_DISCONNECTED:
            transport_close();
            loopFlag = false;
            break; //End break for Q_DISCONNECTED

        //Used when the ping timer has expired and the client has sent a PingReq to the Gateway.
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
                    timer_PingReq = now;
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
            puts("PingResp read");
            //Reset the number of retries and timer.
            retries = 0;
            timer_PingReq = now;
            event->eventID = Q_CONNECTED;
            break; //End break for Q_CLIENT_PING
        
        //Entered when the client receives a PingReq from the Gateway.
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
            break; //End break for Q_SERVER_PING
        
        //Entered into after unsubscribing from a topic name or ID. Expecting an UnsubAck message from the Gateway.
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
            break; //End break for Q_UNSUBSCRIBE
    }//End switch

}// End while loop
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//end client_machine



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
    //Should be set to the port that the Gateway is listening on.
    testClient.destinationPort = 10000;
    //Should be set to the IP address of the Gateway.
    testClient.host = "10.0.2.15";
    testClient.clientID = "ClientSub1";
    //Number of topics the client is subscribed to (excluding wildcards)
    testClient.subscribe_Num = 0;
    //Number of topics the client can publish to.
    testClient.publish_Num = 0;
    //Indicates if the client is subscribed to a topic with a wildcard.
    testClient.wildcard_Sub = false;
    //Number of topics with wildcard the client is subscribed to.
    testClient.sub_Wild_Num = 0;
    //Send out a connect message.
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

    returnCode = client_machine(&event);
    returnCodeHandler(returnCode);
    return 0;
}

