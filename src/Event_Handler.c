/**
 * The event handler for the MQTT-SN Client
 * 
 */

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "Events.h"
#include "Event_Handler.h"
#include "Util.h"
#include "ErrorCodes.h"
#include "transport.h"
#include "Connect.h"
#include "WillTopic.h"
#include "WillMsg.h"
#include "Register.h"
#include "PubAck.h"
#include "Disconnect.h"
#include "PubRecRelComp.h"
#include "PingResp.h"
#include "PingReq.h"

int event_handler(Client_Event *event)
{

    //All these variables should be defined outside the loop.
    bool loopFlag = true;
    int returnCode = Q_ERR_Unknown;

    //The maximum number of times certain message will be attempted to be retransmitted.
    uint8_t maxRetry = 3;
    //The current number of times a message has been retransmitted out. This is set when the message is first sent out.
    uint8_t retries;
    //Used to represent flag variables for any messages that might need them.
    MQTTSNFlags flags;
    MQTTSNString clientString;
    //How often a pingreq message should be sent in seconds.
    size_t ping_req_timeout = event->duration - 5;
    //How long the client is asleep for in seconds.
    size_t sleep_timeout = 15;
    //Indicates if the client will be going back to sleep.
    bool sleepFlag = false;
    //Used to keep track of how much time has past since the last pingreq.
    //Starts when the client has connected.
    time_t timer_PingReq = (time_t) -1;
    //Starts once the client goes to sleep.
    time_t timer_Sleep = (time_t) -1;

while(loopFlag)
{
    flags.all = 0;

    switch(event->eventID){
        case CONNECTING:
            //Check if a message was received from the server.
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode == Q_MsgPending) {
                //Read the message
                returnCode = readMsg(event);
                //If it is a Connack message, move into the Q_CONNECTED state.
                if(returnCode == Q_ConnackRead) {
                    puts("Client connected.");
                    event->eventID = Q_CONNECTED;
                    timer_PingReq = time(NULL);
                    break;
                //If it is a WillTopicReq message, send a WillTopic message and 
                //move into the Q_WILL_TOP_REQ state.
                } else if(returnCode == Q_WillTopReq) {
                    //Need to send a WillTopic message.
                    //Change the # symbol
                    char *willTopic = "Client_#/Will";
                    if(MQTTSNStrCreate(&clientString, willTopic) != Q_NO_ERR) {
                        puts("Error with string creation for WillTopic.");
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    //Send a WillTopic message.
                    returnCode = willTopic(event->client, flags, clientString);
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with sending WillTopic message.");
                        returnCodeHandler(returnCode);
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
                puts("No message received, check connection settings.");
                event->eventID = Q_DISCONNECTED;
                break;
            //Unknown error.
            } else {
                puts("Error with readMsg in CONNECTING State.");
                printf("ReturnCode: %d\n", returnCode);
                event->eventID = Q_DISCONNECTED;
                break;
            }

        case Q_CONNECTED:
            //First check if a pingreq needs to be sent.
            if(time(NULL) - timer_PingReq > 15) {
                //Create an MQTTSNString containing the an empty string so the clientID is not sent over.
                MQTTSNStrCreate(&clientString, NULL);
                //Send the pingReq.
                returnCode = pingReq(event->client, clientString);
                event->eventID = Q_CLIENT_PING;
                break;
            }
            //Checks if a sleeping client needs to wake up and check for messages.
            if(timer_sleep != (time_t) -1){
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
                returnCode = regAck(client, event->topicID, event->msgID, MQTTSN_RC_ACCEPTED);
                if(returnCode != Q_NO_ERR) {
                    puts("Sending of RegAck accept failed.");
                    returnCode = disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECT;
                    break;
                }
                break;
            }
            //Register message received with invalid topicID so send a RegAck with a rejection return code.
            if(returnCode == Q_RejectReg){
                returnCode = regAck(client, event->topicID, event->msgID, MQTTSN_RC_REJECTED_INVALID_TOPIC_ID);
                if(returnCode != Q_NO_ERR) {
                    puts("Sending of RegAck reject failed.");
                    returnCode = disconnect(event->client, 0);
                    event->eventID = Q_DISCONNECT;
                    break;
                }
                break;
            }
            //ANY message to be sent should be placed in this area.
            break;
        
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
            if(timer(NULL) - timer_Sleep < sleep_timeout - 5){
                break;
            }
            //Send a pingReq message to wake the client from sleep.
            MQTTSNStrCreate(&clientString, event->client->clientID);
            returnCode = pingReq(event->client, clientString);
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
                break;
            }
            returnCode = readMsg(event);
            if(returnCode == Q_PubAckRead){
                puts("PubAck received.");
                event->eventID = Q_CONNECTED;
                break;
            } else if (returnCode == Q_PubRecRead){
                puts("PubRec received.");
                returnCode = pubRecRelComp(event->client, event->msgID, MQTTSN_PUBREL);
                if(returnCode != Q_NO_ERR){
                    puts("Error: Failed to send PubRel");
                    break;
                }
                event->eventID = Q_PUB_QOS2;
                break;
            } else {
                puts("Error with publish qos");
                event->eventID = Q_CONNECTED;
                break;
            }

        case Q_PUB_QOS2:
            returnCode = msgReceived(event->client->mySocket);
            if(returnCode != Q_MsgPending) {
                puts("Error: No message for publish qos2");
                event->eventID = Q_CONNECTED;
                break;
            }
            returnCode = readMsg(event);
            if(returnCode != Q_PubCompRead){
                ("Error with receiving pubComp");
                event->eventID = Q_CONNECTED;
                break;
            }
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
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    //Send a WillMsg
                    returnCode = willMsg(event->client, clientString);
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with sending WillMsg message.");
                        returnCodeHandler(returnCode);
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    //Transition to the Q_WILL_MSG_REQ state.
                    event->eventID = Q_WILL_MSG_REQ;
                    break;
                } else {
                    puts("Unexpected error/message in Q_WILL_TOP_REQ");
                    event->eventID = Q_DISCONNECTED;
                    break;
                }
            } else {
                puts("No message received.");
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
            event->eventID = Q_CONNECTED;
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
                    returnCode = pingReq(event->client, clientString);
                    //If unsuccessful, shutdown the client by going to disconnected state.
                    if(returnCode != Q_NO_ERR) {
                        puts("Error with ping transmission.");
                        event->eventID = Q_DISCONNECTED;
                        break;
                    }
                    //Transition back to the Q_CLIENT_PING state to check if a pingresp came.
                    event->eventID = Q_CLIENT_PING;
                    break;
                //Close off the socket
                } else {
                    put("Max ping retries reached.");
                    event->eventID = Q_DISCONNECTED;
                    returnCode = Q_ERR_NoPingResp;
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
            returnCode = msgReceived(client);
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

}
exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}

