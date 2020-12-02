/**
 * Percentage Contribution: Sandeep Bindra (70%), Evon Choong (30%)
 * Sleeping MQTT-SN Client
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
 * Represents a sleeping client. 
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
    //Used to represent the flag section for any messages that might need them.
    MQTTSNFlags flags;
    //This struct is needed for certain portions of some messages by
    //the PAHO library.
    MQTTSNString clientString;

    //How long the client is asleep for in seconds.
    int sleep_timeout = 15;

    //Indicates if the client will be going to sleep.
    bool sleepFlag = false;
    //Tells the client to go back into sleep after awaking.
    bool sleepReturn = false;
    //Indicates if the client will need to subscribe to any topics.
    bool subscribeFlag = true;
    bool sub0 = true;
    //Starts once the client goes to sleep.
    time_t timer_Sleep = (time_t) -1;
    
//Make the client continiously loop through its various states
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
                    //timer_PingReq = now;
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
            //Something might be wrong with connection settings, have the client come back
            //to this state 2 more times and if no message has been received still, show a message
            //indicating connection settings should be checked.
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
                
            //Unknown error, go to disconnected.
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
            if(subscribeFlag) {
                if(sub0) {
                    //Indicate a normal topic name
                    flags.bits.topicIdType = 0b00;
    
                    //The topic name the client will subscribe to
                    MQTTSN_topicid topic; 

                    topic.type = MQTTSN_TOPIC_TYPE_NORMAL;
                    //Name of the topic to subscribe to
                    topic.data.long_.name = "PubSubClient/Test/Checking";

                    //Obtain the length of the topic name and assign it to the len member of MQTTSN_topicid 
                    int topicLen = (int)strlen(topic.data.long_.name);
                    if(topicLen > 0) {
                        topic.data.long_.len = (size_t)topicLen;
                    }
                    //Subscribe to QoS level 1 (this is irrelevant, however, 
                    //because the publisher dictates the QoS level with the Gateway being used)
                    flags.bits.QoS = 0b01;
                    //Send out the subscribe message and check if it was successful.
                    returnCode = subscribe(event->client, &topic, flags, event->send_msgID);
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with subscribing.");
                        returnCodeHandler(returnCode);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    event->qos = flags.bits.QoS;
                    //Make these variables false to ensure the client does not try subscribing again.
                    sub0 = false;
                    subscribeFlag = false;
                    //Transition to Q_Subscribing 
                    event->eventID = Q_SUBSCRIBING;
                    break;
                }
                break;
            } //End if(subscribeFlag)

            //Check if there are any messages to be read.
            if(msgReceived(event->client->mySocket) != Q_MsgPending) {
                //Check if the client should go to sleep.
                if(sleepFlag) {
                    returnCode = disconnect(event->client, (uint16_t)sleep_timeout);
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with sending disconnect for client to sleep.");
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    event->eventID = Q_DISCONNECTING;
                    break;
                //Client woke up from sleep and should now return to sleep.    
                } else if(sleepReturn) {
                    event->eventID = Q_SLEEP;
                    printf("%s%d%s\n", "Going back to sleep for: ", sleep_timeout, " seconds.");
                    timer_Sleep = now;
                    break;
                } else {
                    break;
                }
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
                //Send a PubRec message.
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
            if(returnCode == Q_RejectReg) {
                returnCode = regAck(event->client, event->topicID, event->msgID, MQTTSN_RC_REJECTED_INVALID_TOPIC_ID);
                if(returnCode != Q_NO_ERR) {
                    puts("Sending of RegAck reject failed.");
                    returnCode = disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
                break;
            }
            //If the client receives a PingResp, it does nothing, unless it is a sleeping client,
            //in which case, the PingResp indicates it can return to sleep.
            if(returnCode == Q_PingRespRead) {
                puts("PingResp read.");
                if(sleepReturn){
                    printf("%s%d%s\n", "Going back to sleep for: ", sleep_timeout, " seconds.");
                    event->eventID = Q_SLEEP;
                    timer_Sleep = now;
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
            //QoS level 2 message exchange is complete so cycle back to Q_Connected state.
            event->eventID = Q_CONNECTED;
            break; //End break for Q_RCV_QOS2

        //Used when the client is sleeping
        case Q_SLEEP:
            //If the sleep timer is not over, continuously cycle back to this state.
            if(now - timer_Sleep < sleep_timeout - 5) {
                break;
            }
            //If this is the first time the client has woken up, it won't need to send
            //another disconnect message everytime it goes back to sleep.
            if(sleepFlag) {
                sleepFlag = false;
                sleepReturn = true;
            }
            //Send a pingReq message to wake the client from sleep.
            MQTTSNStrCreate(&clientString, event->client->clientID);
            returnCode = pingReq(event->client, &clientString);
            if(returnCode != Q_NO_ERR) {
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
            //Check the acknowledgement message received. If it is a pubAck, this was Qos level 1 and if
            //it is a pubRel, this was Qos level 2. 
            returnCode = readMsg(event);
            if(returnCode == Q_PubAckRead){
                //Increment the msgID for the next message that gets sent out.
                event->send_msgID = (uint16_t)(event->send_msgID + 1);
                puts("PubAck received.");
                //Reset the timer. Only used for a client that publishes.
                //timer_Publish = now;
                event->eventID = Q_CONNECTED;
                break;
            } else if (returnCode == Q_PubRecRead){
                puts("PubRec received.");
                //PubRec received so client will send out a pubRel.
                returnCode = pubRecRelComp(event->client, event->send_msgID, MQTTSN_PUBREL);
                if(returnCode != Q_NO_ERR){
                    puts("Error: Failed to send PubRel");
                    //Reset the timer.
                    //timer_Publish = now;
                    break;
                }
                event->eventID = Q_PUB_QOS2;
                break;
            } else {
                puts("Error with publish qos");
                //Reset the timer, only used if publishing.
                //timer_Publish = now;
                event->eventID = Q_CONNECTED;
                break;
            }
            break; //End break for Q_PUBLISH

        //Used when the client has sent out a publish message with Qos level 2.
        case Q_PUB_QOS2:
            //Client will be expecting back a pubComp message.
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
            //Check if a message has been received.
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode == Q_MsgPending) {
                returnCode = readMsg(event);
                //Check if a WillMsgReq has been received.
                if(returnCode == Q_WillMsgReq) {
                    //A placeholder, can be changed to whatever the desired name is for the Will Topic
                    char *msg = "Client_# down.";
                    MQTTSNStrCreate(&clientString, msg);
                    //Send a WillMsg and check if was successfully sent.
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
            //Expecting a Connack message.
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode == Q_MsgPending) {
                returnCode = readMsg(event);
                if(returnCode != Q_ConnackRead){
                    puts("Unexpected error/message in Q_WILL_MSG_REQ");
                    //Break the loop and indicate could not connect to server.
                    disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
                //WillMsg has been acknowledged so client is now connected.
                event->eventID = Q_CONNECTED;
                //Following variable is only applicable if client plans on no longer going 
                //to sleep and wants to stay in a active, connected state.
                //timer_PingReq = now;
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
            //Increment the msgID for the next message that gets sent out. 
            event->send_msgID = (uint16_t)(event->send_msgID + 1);
            break; //End break for Q_REGISTERING

        //Used when subscribing to a topic.
        case Q_SUBSCRIBING:
            //Expecting a SubAck message with a returncode of Accepted.
            returnCode = msgReceived(event->client->mySocket);
            //First check if a message has been received.
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
            //If a SubAck with an accepted return code was received, allow the client to enter into sleep.
            //This configuration is only for a client that will be going to sleep.
            sleepFlag = true;
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
            //If this is a sleeping client, send it to the Q_SLEEP state.
            if(sleepFlag) {
                event->eventID = Q_SLEEP;
                timer_Sleep = now;
                printf("%s%d%s\n", "Entering sleep for: ", sleep_timeout, " seconds.");
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
                    //timer_PingReq = now;
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
            //timer_PingReq = now;
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
    testClient.clientID = "SleepingClient";
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

