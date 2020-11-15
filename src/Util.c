/**
 * Serves as a utility file for any extra functions that will be needed
 * by the MQTT-SN messages.
 */ 

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>

#include "Client_t.h"
#include "Events.h"
#include "ErrorCodes.h"
#include "MQTTSNPacket.h"
#include "Util.h"
#include "StackTrace.h"
#include "transport.h"
#include "MQTTSNConnect.h"
#include "MQTTSNUnsubscribe.h"
#include "MQTTSNSubscribe.h"
#include "MQTTSNPublish.h"
#include "PubAck.h"
#include "PubRecRelComp.h"

#define Q_BUF_LEN 1600

/**
 * This function will be used to create an MQTTSNString, which is needed to create a WillTopic message and WillMsg,
 * @param strContainer A pointer to a MQTTSNString struct that the data will be written to.
 * @param string The character string that will be written into the passed MQTTSNString struct.
 * @return The error code. Either Q_NO_ERR (0), which is a success, or Q_ERR_StrCreate (13).
 */
int MQTTSNStrCreate(MQTTSNString *strContainer, char *string)
{
    int returnCode = Q_ERR_StrCreate;

    FUNC_ENTRY;

    //We want to create an MQTTSNString struct that contains no strings. 
    //Will need this for PingReq message when we don't want to specify a clientID.
    if(string == NULL) {
        strContainer->cstring = NULL;
        strContainer->lenstring.data = NULL;
        strContainer->lenstring.len = 0;
    }

    //TODO Ask Lawrence if we want to set this cstring member because the code determines its length using strlen in 
    //MQTTSNPacket using the MQTTSNstrlen function
    strContainer->cstring = string;
    strContainer->lenstring.data = string;
    //size_t strSize = sizeof(strContainer->lenstring.data);
    //TODO Ask Lawrence if this is ok.
    size_t strLength = strlen(strContainer->lenstring.data);
    strContainer->lenstring.len = strLength;

    returnCode = Q_NO_ERR;

    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End MQTTSNStrCreate


/**
 * 
 * OLD CODE
 * @param clientPtr The Client that is receiving a Publish message with a certain QoS level.
 * @param QoS The Quality of Service level for the Publish message received.
 * @param msgID The message ID of the Publish message received.
 * @param topicID Contains the message topicID
 * @return An int: Q_NO_ERR (0) indicates the procedure for the indicated QoS level was satisfied. Otherwise, Q_ERR_Unknown,
 * Q_ERR_PubAck, Q_ERR_PubComp, Q_ERR_MsgID, Q_ERR_Deserial, Q_ERR_MsgType, Q_ERR_PubRec, Q_ERR_QoS indicate an error.
 
int qosResponse(Client_t *clientPtr, const uint8_t QoS, const uint16_t msgID, const uint16_t topicID)
{
    int returnCode = Q_ERR_Unknown;
    //Buffer needed if deserializing 
    unsigned char buf[Q_BUF_LEN] = {0};
    size_t bufSize = sizeof(buf);
    //Need to check the QoS level of the message to determine how the client should respond.
    //For QoS level 1, the client needs to send a PubAck message back to the server so it knows
    //it got the message.

    //DEBUG
    //printf("%s%u\n", "Qos: ", QoS);

    FUNC_ENTRY;
    //If QoS level is 0, do nothing.
    if(QoS == 0){
        returnCode = Q_NO_ERR;
        goto exit;
    }

    else if(QoS == 1)
    {
        //The accepted return code
        uint8_t msgRtrnCode = 0;
        if(pubAck(clientPtr, topicID, msgID, msgRtrnCode) != Q_NO_ERR)
        {
            returnCode = Q_ERR_PubAck;
            goto exit;
        }
    }
    //For QoS level 2, the client needs to go through a series of acknowledgement messages.
    else if(QoS == 2)
    {
        //Need to send out a PubRec message first. Check to make sure it went through.
        if(pubRecRelComp(clientPtr, msgID, MQTTSN_PUBREC) != Q_NO_ERR)
        {
            int msgCheck = msgReceived(clientPtr->mySocket, timer);
            if(msgCheck == Q_MsgPending)
            {
                //Will have to read in a PubRel message. Check to make sure it was received.
                if(MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_PUBREL)
                {
                    //Says what kind of message this is.
                    unsigned char *msgType = NULL;
                    //Will be used to ensure the MsgID has stayed the same 
                    //as in the original publish message received.
                    uint16_t msgID2 = 0;
                    if(MQTTSNDeserialize_ack(msgType, &msgID2, buf, bufSize) == 1)
                    {
                        if(msgID2 == msgID)
                        {
                            if(pubRecRelComp(clientPtr, msgID, MQTTSN_PUBCOMP) != Q_NO_ERR)
                            {
                                returnCode = Q_ERR_PubComp;
                                goto exit;
                            }
                        }
                        else
                        {
                            returnCode = Q_ERR_MsgID;
                            goto exit;
                        }
                    }
                    else
                    {
                        returnCode = Q_ERR_Deserial;
                        goto exit;
                    }
                }
                else
                {
                    returnCode = Q_ERR_MsgType;
                    goto exit;
                }
            }
            else
            {
                returnCode = Q_ERR_NoPubRel;
                goto exit;
            }
        }
        else
        {
            returnCode = Q_ERR_PubRec;
            goto exit;
        }
    }
    //An unknown QoS level has been passed. 
    //This should not happen, but this is for safety.
    else
    {
        returnCode = Q_ERR_QoS;
        goto exit;
    }
    
exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End qosResponse
*/

/**
 * Helper function to deserialize a CONNACK message and check its return type.
 */ 
int readConnack(unsigned char *buf, size_t bufSize)
{
    int returnCode = Q_ERR_Unknown;

    FUNC_ENTRY;
    //Check if the deserialization is unsuccessful
    if(MQTTSNDeserialize_connack(&returnCode, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Check if the return code value of the message is accepted.
    if(returnCode == MQTTSN_RC_ACCEPTED){
        returnCode = Q_NO_ERR;
        goto exit;
    } else{
        returnCode = Q_ERR_Rejected;
        goto exit;
    }

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End readConnack

int readWillTopResp(unsigned char *buf, size_t bufSize)
{

    int returnCode = Q_ERR_Unknown;
    FUNC_ENTRY;
    //Check if deserialization is successful.
    if(MQTTSNDeserialize_willtopicresp(&returnCode, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Check if the return code value of the message is accepted.
    if(returnCode == MQTTSN_RC_ACCEPTED){
        returnCode = Q_NO_ERR;
        goto exit;
    } else{
        returnCode = Q_ERR_Rejected;
        goto exit;
    }

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End readWillTopResp

/**
 * 
 * 
 */ 
int readWillMsgResp(unsigned char *buf, size_t bufSize)
{

    int returnCode = Q_ERR_Unknown;
    FUNC_ENTRY;
    //Check if deserialization is successful.
    if(MQTTSNDeserialize_willmsgresp(&returnCode, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Check if the return code value is accepted for this message.
    if(returnCode == MQTTSN_RC_ACCEPTED){
        returnCode = Q_NO_ERR;
        goto exit;
    } else{
        returnCode = Q_ERR_Rejected;
        goto exit;
    }

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End readWillMsgResp

/**
 * 
 * @param buf
 * @param bufSize
 * @param event
 * @return
 */ 
int readUnsubAck(unsigned char *buf, size_t bufSize, Client_Event_t *event)
{

    int returnCode = Q_ERR_Unknown;
    //Stores the messageID of the unsuback message.
    uint16_t ack_msgID = 0;

    FUNC_ENTRY;
    //Check if deserialization is successful
    if(MQTTSNDeserialize_unsuback(&ack_msgID, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Make sure the msgID of the unsuback message matches the msgID of the sent subscribe message
    if(ack_msgID != event->send_msgID){
        returnCode = Q_ERR_MsgID;
        goto exit;
    } else{
        returnCode = Q_NO_ERR;
        goto exit;
    }

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End readUnsubAck

/**
 * 
 * @param buf
 * @param bufSize
 * @param event
 * @return
 */ 
int readSubAck(unsigned char *buf, size_t bufSize, Client_Event_t *event)
{

    int returnCode = Q_ERR_Unknown;

    //Following variables are used to store the values contained within
    //the unsuback message sent by the server.
    int ack_qos = 0;
    uint16_t ack_topicID = 0;
    uint16_t ack_msgID = 0;
    //Stores the return code contained in the message
    uint8_t ack_Return = 0;
    
    FUNC_ENTRY;
    //Check if deserialization if successful.
    if(MQTTSNDeserialize_suback(&ack_qos, &ack_topicID, &ack_msgID, &ack_Return, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Check if the return code value of the message is accepted.
    if(ack_Return != MQTTSN_RC_ACCEPTED){
        returnCode = Q_ERR_Rejected;
        goto exit;
    }
    //Check that the msgID and qos level match.
    if(event->send_msgID != ack_msgID){
        returnCode = Q_ERR_MsgID;
        goto exit;
    }
    if(event->qos != ack_qos){
        returnCode = Q_ERR_Qos;
        goto exit;
    }
    //Check if this was a wildcard subscription.
    //If not, assign the new topicID to the array of the client's subscribed topics.
    if(ack_topicID != 0) {
        //Number of topics the client is subscribed to will increase by one.
        event->client->subscribe_Num += 1;
        //Get the index for this new topic to be added in for the array of the client's subscribed topics.
        size_t subIndex = event->client->subscribe_Num - 1;
        event->client->sub_topicID[subIndex] = ack_topicID;
    } else {
        //Increment the number of wildcard topics the client is subscribed to.
        event->client->sub_Wild_Num += 1;
        //The client now has a wildcard subscription.
        event->client->wildcard_Sub = true;
    }

    returnCode = Q_NO_ERR;
    goto exit;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End readSubAck



/**
 * 
 * Used to read in a publish message for a subscribed client.
 * @param 
 * @param
 * @param 
 * @return An int
 */ 
int readPub(unsigned char *buf, size_t bufSize, Client_Event_t *event)
{
    int returnCode = Q_ERR_Unknown;

    //Used to hold flags and data of the Publish message
    unsigned char dup = 0;
    int qos = 0;
    unsigned char retained = 0;
    unsigned short pubMsgID = 0;
    MQTTSN_topicid pubTopic;
    unsigned char *pubData = 0;
    int dataLen = 0;

    FUNC_ENTRY;
    //Check if deserialization was successful.
    if(MQTTSNDeserialize_publish(&dup, &qos, &retained, &pubMsgID, &pubTopic, &pubData, &dataLen, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }

    //Stores the topicID contained in the publish message.
    uint16_t pubTopicID = pubTopic.data.id;

    //Check if the topicID of the message matches with a wildcard subscription of the client's.
    if(event->client->wildcard_Sub && pubTopicID == event->client->wild_topicID){
        //Display the data in the publish message.
        for(int index = 0; index < dataLen; ++index){
            printf("%c", pubData[index]);
            //Print a new line once this is the end of the data to be printed.
            if(index == (dataLen - 1)){
                puts("");
            }
        }
    //Check if the client has any subscriptions to topics without wildcards.
    } else if(event->client->subscribe_Num > 0) {
        //Indicates if the client is subscribed to this topic.
        bool subscribed = false;
        //Check if the topicID of the message matches with a topicID the client is subscribed to.
        for(size_t index = 0; index < event->client->subscribe_Num; ++index){
            if(pubTopicID == event->client->sub_topicID[index]){
                subscribed = true;
                break;
            }
        }
        //Display the data in the publish message if the client is subscribed to this topic.
        if(subscribed){
            for(int index = 0; index < dataLen; ++index){
                printf("%c", pubData[index]);
                //Print a new line once this is the end of the data to be printed.
                if(index == (dataLen - 1)){
                    puts("");
                }
            }
        //Client is not subscribed to this topicID
        } else{
            returnCode = Q_ERR_WrongTopicID;
            //Need this information when sending a pubAck for rejection.
            event->topicID = pubTopicID;
            event->msgID = pubMsgID;
            goto exit;
        }
    //Client is not subscribed to this topicID
    } else{
        returnCode = Q_ERR_WrongTopicID;
        //Need this information when sending a pubAck for rejection.
        event->topicID = pubTopicID;
        event->msgID = pubMsgID;
        goto exit;
    }

    //Check the qos levels to return the appropriate return so the client knows how to respond.
    if(qos == 1){
        returnCode = Q_pubQos1;
        //Will need the msgID and topicID for the pubAck message that needs to be sent.
        event->msgID = pubMsgID;
        event->topicID = pubTopicID;
        goto exit;
    } else if (qos == 2){
        returnCode = Q_pubQos2;
        //Will need the msgID for the pubRec and pubComp message that need to be sent.
        event->msgID = pubMsgID;
        goto exit;
    } else{
        returnCode = Q_pubQos0;
        goto exit;
    }

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End readPub

/**
 * 
 * @param buf
 * @param bufSize
 * @param event
 * @return
 */ 
int readReg(unsigned char *buf, size_t bufSize, Client_Event_t *event)
{
    int returnCode = Q_ERR_Unknown;

    //Following variables are used to store values contained within the Register message.
    uint16_t regTopicID = 0;
    uint16_t regMsgID = 0;
    MQTTSNString topicName;

    FUNC_ENTRY;
    //Check that deserialization is successful.
    if(MQTTSNDeserialize_register(&regTopicID, &regMsgID, &topicName, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }

    //If the client reconnected with the cleanSession flag off, then check if it 
    //has any subscriptions (excluding wildcards) and then check if it supposed to be subscribed to the topic mentioned.
    if(event->client->subscribe_Num > 0){
        for(size_t index = 0; index < event->client->subscribe_Num; ++index){
            if(regTopicID == event->client->sub_topicID[index]){
            //The msgID and topicID to be used by the client when it sends the regAck message.
            event->msgID = regMsgID;
            event->topicID = regTopicID;
            returnCode = Q_Subscribed;
            goto exit;
            }
        }
    //Check if the client has any wildcard subscriptions    
    } else if (event->client->wildcard_Sub){
        //client will need this topicID when it checks the publish messages.
        event->client->wild_topicID = regTopicID;
        //The msgID and topicID of the register message will need to be used by the client when it sends the regAck message.
        event->msgID = regMsgID;
        event->topicID = regTopicID;
        returnCode = Q_Wildcard;
        goto exit;
    //Otherwise, the client needs to send a regack message rejecting this topicID.
    } else{
        returnCode = Q_RejectReg;
        event->msgID = regMsgID;
        event->topicID = regTopicID;
        goto exit;
    }

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//end readReg

int readRegAck(unsigned char *buf, size_t bufSize, Client_Event_t *event)
{
    int returnCode = Q_ERR_Unknown;

    uint8_t ack_Return = 0;
    uint16_t ack_topicID = 0;
    uint16_t ack_msgID = 0;
    
    FUNC_ENTRY;
    //Check if deserialization was successful
    if(MQTTSNDeserialize_regack(&ack_topicID, &ack_msgID, &ack_Return, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Check if the return code value is accepted.
    if(ack_Return != MQTTSN_RC_ACCEPTED){
        returnCode = Q_ERR_Rejected;
        goto exit;
    }
    //Check that the messageID of the RegAck message matches with the one from the client's register message.
    if(ack_msgID != event->send_msgID){
        returnCode = Q_ERR_MsgID;
        goto exit;
    }

    //Index for this topicID to be stored.
    size_t pubIndex = event->client->publish_Num;
    //Increment the number of topicIDs the client has to publish to.
    event->client->publish_Num += 1;
    //Store the topicID given by the Server
    event->client->pub_topicID[pubIndex] = ack_topicID;
    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//end readRegAck

int readPubAck(unsigned char *buf, size_t bufSize, Client_Event_t *event)
{
    int returnCode = Q_ERR_Unknown;

    //Following variables are needed to store corresponding values contained within the message.
    uint16_t ack_topicID = 0;
    uint16_t ack_msgID = 0;
    uint8_t ack_return = 0;

    //Check if deserialization is successful
    if (MQTTSNDeserialize_puback(&ack_topicID, &ack_msgID, &ack_return, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Check if the message has a return code value of accepted.
    if(ack_return != MQTTSN_RC_ACCEPTED){
        returnCode = Q_ERR_Rejected;
        goto exit;
    }
    //Check that the msgID matches with that of the corresponding publish message.
    if(ack_msgID != event->send_msgID){
        returnCode = Q_ERR_MsgID;
        goto exit;
    }
    //Check that the topicID matches with that of the corresponding publish message.
    if(ack_topicID != event->topicID){
        returnCode = Q_ERR_WrongTopicID;
        goto exit;
    }

    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End readPubAck


/**
 * 
 * @param buf
 * @param bufSize
 * @param msgType
 * @return 
 */ 
int readRecRelComp(unsigned char *buf, size_t bufSize, Client_Event_t *event, unsigned char msgType)
{
    int returnCode = Q_ERR_Unknown;
    uint16_t ack_msgID = 0;

    //Check if message deserialization is successful
    if(MQTTSNDeserialize_ack(&msgType, &ack_msgID, buf, bufSize) != 1){
        returnCode = Q_ERR_Deserial;
        goto exit;
    }
    //Check if the msgID matches with the msgID of the sent publish message.
    if(ack_msgID != event->send_msgID){
        returnCode = Q_ERR_MsgID;
        goto exit;
    }
    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;

}//end readRecRelComp


/**
 * Checks if there is a message that has been sent by the server.
 * @param clientSock
 * 
 */
int msgReceived(int clientSock)
{
    int returnCode = Q_ERR_Unknown;

    //time_t secs = 1;
    //Determines how many microseconds the client will wait for a message.
    suseconds_t micro_sec = 400000;

    FUNC_ENTRY;
    //Will be used to check the return value of select which is the number of file descriptors (FDs) 
    //returned, 0, or -1 if there is an error.
    int numFD_Rtrn = 0;
    //The set of file descriptors (FDs) to be checked 
    //if any data has been received on.
    fd_set readSet;

    //Clears out the set.
    FD_ZERO(&readSet);

    //The maximum number of FDs to be read.
    //This value must be set to the highest numbered FD plus 1.
    int maxFDP1 = clientSock + 1;

    //Place our socket in the FD set for reading FDs.
    FD_SET(clientSock, &readSet);

    //Determines how long the client will wait for the server to send a message.
    struct timeval timer = {0, micro_sec};

    //Check if the client's socket has received any data.
    numFD_Rtrn = select(maxFDP1, &readSet, NULL, NULL, &timer);
    if(numFD_Rtrn > 0 && FD_ISSET(clientSock, &readSet)){
        returnCode = Q_MsgPending;
        goto exit;
    }
    else{
        returnCode = Q_NoMsg;
        goto exit;
    }
    
exit:    
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}//End msgReceived


/**
 * 
 * @param event
 * @return 
 */ 
int readMsg(Client_Event_t *event)
{
    
    int returnCode = Q_ERR_Unknown;

    //Buffer used to read in a message
    unsigned char buf[Q_BUF_LEN] = {0};
    //The size of the buffer.
    size_t bufSize = sizeof(buf);
    int msgType = 0;
    FUNC_ENTRY;
    msgType = MQTTSNPacket_read(buf, bufSize, transport_getdata);

    //Check if the message has a length that is longer than the size allowed by the buffer.
    //If it is, return an error code indicating the message is rejected due to exceeding the
    //maximum allowed size.
    if(buf[0] == 1) {
        int packetLength = 0;
        //The next two bytes determine the packet length so they will be read in as a 16 byte number.
        unsigned char *len = (buf+1);
        packetLength = readInt(&len);
        if(packetLength > Q_BUF_LEN) {
            returnCode = Q_ERR_MaxLength;
            returnCodeHandler(returnCode);
            goto exit;
        }    
    }
    
    //Read operation failed.
    if(msgType == MQTTSNPACKET_READ_ERROR) {
        returnCode = Q_ERR_Read;
        returnCodeHandler(returnCode);
        goto exit;
    } else {
        //Following switch statement checks the type of message, deserializes it (if necessary), 
        //and returns the appropriate returnCode with regards to any processing of that message.
        switch (msgType){
            case MQTTSN_CONNACK:
                returnCode = readConnack(buf, bufSize);
                //Check if the Connack indicated an accepted value for the return code.
                if(returnCode == Q_NO_ERR) {
                    returnCode = Q_ConnackRead;
                    break;
                }
                //Otherwise, display the error code.
                returnCodeHandler(returnCode);
                break;

            //No need to deserialize the WillTopicReq or WillMsgReq messages, as there is nothing to be checked.
            case MQTTSN_WILLTOPICREQ:
                returnCode = Q_WillTopReq;
                break;

            case MQTTSN_WILLMSGREQ:
                returnCode = Q_WillMsgReq;
                break;

            case MQTTSN_WILLTOPICRESP:
                returnCode = readWillTopResp(buf, bufSize);
                //Check if the WillTopResp indicated an accepted value for the return code.
                if(returnCode == Q_NO_ERR) {
                    returnCode = Q_TopicRespRead;
                    break;
                }else {
                    //Otherwise display the error code.
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_WILLMSGRESP:
                returnCode = readWillMsgResp(buf, bufSize);
                //Check if the WillMsgResp indicated an accepted value for the return code.
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_MsgRespRead;
                    break;
                //Otherwise display the error code.
                }else{
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_PINGRESP:
                //No need to deserialize the PingResp message
                returnCode = Q_PingRespRead;
                break;

            case MQTTSN_DISCONNECT:
                //No need to deserialize the disconnect message
                returnCode = Q_DisconRead;
                break;

            case MQTTSN_PINGREQ:
                //No need to deserialize the PingReq message
                returnCode = Q_PingReqRead;
                break;

            case MQTTSN_UNSUBACK:
                returnCode = readUnsubAck(buf, bufSize, event);
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_UnsubackRead;
                    break;
                //Display the error code.
                } else{
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_SUBACK:
                returnCode = readSubAck(buf, bufSize, event);
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_SubAckRead;
                    break;
                } else{
                    returnCodeHandler(returnCode);
                    break;
                }
            
            case MQTTSN_REGISTER:
                returnCode = readReg(buf, bufSize, event);
                if(returnCode != Q_ERR_Deserial && returnCode != Q_ERR_Unknown){
                    break;
                }else{
                    returnCodeHandler(returnCode);
                    break;
                }
            
            case MQTTSN_REGACK:
                returnCode = readRegAck(buf, bufSize, event);
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_RegAckRead;
                    break;
                } else {
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_PUBACK:
                returnCode = readPubAck(buf, bufSize, event);
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_PubAckRead;
                    break;
                } else {
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_PUBREC:
                returnCode = readRecRelComp(buf, bufSize, event, MQTTSN_PUBREC);
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_PubRecRead;
                    break;
                } else{
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_PUBREL:
                returnCode = readRecRelComp(buf, bufSize, event, MQTTSN_PUBREL);
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_PubRelRead;
                    break;
                } else{
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_PUBCOMP:
                returnCode = readRecRelComp(buf, bufSize, event, MQTTSN_PUBCOMP);
                if(returnCode == Q_NO_ERR){
                    returnCode = Q_PubCompRead;
                    break;
                } else{
                    returnCodeHandler(returnCode);
                    break;
                }

            case MQTTSN_PUBLISH:
                //Will have to check the returnCode in the event handler to determine what to do next.
                returnCode = readPub(buf, bufSize, event);
                break;

            //If unknown, message type, print out its value.
            default: 
                puts("Unknown message type.");
                printf("MsgType: %d\n",  msgType);
                break;
        }//End Switch
            
    }//end else

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;

}//End readMsg

/**
 * Used to read the return code and print out a response for that return code. Will be used
 * by the test cases.
 * @param returnCode The returnCode from one of the functions called in the process of building or sending
 * an MQTT-SN message.
 * @return void
 */ 
void returnCodeHandler(int returnCode)
{
    switch(returnCode)
    {
        case Q_NO_ERR:
            puts("No error");
            break;
        
        case Q_ERR_Socket:
            puts("Socket error");
            break;

        case Q_ERR_Connect:
            puts("Connection error");
            break; 

        case Q_ERR_Deserial:
            puts("Deserial error");
            break;

        case Q_ERR_Connack:
            puts("Connack Error");
            break;

        case Q_ERR_SocketOpen:
            puts("Socket could not be opened");
            break;

        case Q_ERR_WillTopReq:
            puts("Error: No WillTopicReq from server");
            break;

        case Q_WillTopReq:
            puts("WillTopReq Received");
            break;

        case Q_ERR_Disconnect:
            puts("Disconnect Error");
            break;

        case Q_ERR_Ack:
            puts("Server acknowledgment error");
            break;

        case Q_ERR_CSocket:
            puts("Connect socket error");
            break;

        case Q_ERR_DSocket:
            puts("Disconnect socket error");
            break;

        case Q_ERR_Qos:
            puts("QoS error");
            break;

        case Q_ERR_Retain:
            puts("Retain flag error");
            break;

        case Q_ERR_Serial:
            puts("Serialize error");
            break;

        case Q_ERR_StrCreate:
            puts("StrCreate error");
            break;

        case Q_WillMsgReq:
            puts("WillMsgReq received");
            break;

        case Q_ERR_WillMsgReq:
            puts("Error: No WillMsgReq received from server");
            break;

        case Q_ERR_TopicIdType:
            puts("Invalid Topic ID type");
            break;   

        case Q_ERR_PubAck:
            puts("Error: PubAck not received.");
            break;

        case Q_ERR_MsgID:
            puts("Error: Mismatching message ID");
            break;

        case Q_ERR_Rejected:
            puts("Message Rejected.");
            break;

        case Q_ERR_MsgType:
            puts("Wrong Message Type");
            break;

        case Q_ERR_Unknown:
            puts("Unknown error");
            break;

        case Q_ERR_SubAck:
            puts("Error: No SubAck received from server");
            break;

        case Q_Wildcard:
            puts("WildCard Subscription");
            break;

        case Q_NoMsg:
            puts("No messages for client.");
            break;

        case Q_ERR_WrongTopicID:
            puts("Error: Mismatching topicID");
            break;

        case Q_ERR_PubRec:
            puts("Error: PubRec not sent");
            break;

        case Q_ERR_qosResponse:
            puts("QoS failure");
            break;

        case Q_PubMsgRead:
            puts("Publish message received");
            break;

        case Q_ERR_PubComp:
            puts("Error: PubComp not sent");
            break;

        case Q_ERR_PubRel:
            puts("Error: PubRel not sent");
            break;
        
        case Q_ERR_SendRel:
            puts("PubComp not received");
            break;

        case Q_ERR_RePub:
            puts("PubRec not received");
            break;

        case Q_ERR_NoPingResp:
            puts("PingResp message not received");
            break;
        
        case Q_ERR_NoPubPing:
            puts("No publish/pingresp message received");
            break;

        case Q_ERR_NoPubRel:
            puts("No PubRel message received");
            break;

        case Q_ERR_NoPub:
            puts("No Publish message received");
            break;
        
        case Q_ERR_NoReg:
            puts("No Register message received");
            break;

        case Q_ERR_MaxLength:
            puts("Max packet length received.");
            break;

        case Q_ERR_Read:
            puts("Read error");
            break;

        default:
            puts("Foreign return code");
            break;
    }
}
