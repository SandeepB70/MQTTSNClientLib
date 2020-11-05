/**
 * Serves as a utility file for any extra functions that will be needed
 * by the MQTT-SN messages.
 */ 

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "Client_t.h"
#include "ErrorCodes.h"
#include "MQTTSNPacket.h"
#include "Util.h"
#include "StackTrace.h"
#include "transport.h"
#include "PubAck.h"
#include "PubRecRelComp.h"

/**
 * This function will be used to create an MQTTSNString, which is needed to create a WillTopic message and WillMsg,
 * @param strContainer A pointer to a MQTTSNString struct that the data will be written to.
 * @param string The character string that will be written into the passed MQTTSN struct.
 * @return The error code. Either Q_NO_ERR (0), which is a success, or Q_ERR_StrCreate (13).
 */
int MQTTSNStrCreate(MQTTSNString *strContainer, char *string)
{
    int returnCode = Q_ERR_StrCreate;

    FUNC_ENTRY;

    //We want to create an MQTTSNString struct that contains no strings. 
    //Will need this for PingReq message when we don't want to specify a clientID.
    if(string == NULL)
    {
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
 * 
 * @param clientPtr The Client that needs is receiving a Publish message with a certain QoS level.
 * @param QoS The Quality of Service level for the Publish message received.
 * @param msgID The message ID of the Publish message received.
 * @param topicID Contains the message topicID
 * @return An int: Q_NO_ERR (0) indicates the procedure for the indicated QoS level was satisfied. Otherwise, Q_ERR_Unknown,
 * Q_ERR_PubAck, Q_ERR_PubComp, Q_ERR_MsgID, Q_ERR_Deserial, Q_ERR_MsgType, Q_ERR_PubRec, Q_ERR_QoS indicate an error.
 */
int qosResponse(Client_t *clientPtr, const uint8_t QoS, const uint16_t msgID, const uint16_t topicID)
{
    int returnCode = Q_ERR_Unknown;
    //Buffer needed if deserializing 
    unsigned char buf[1600] = {0};
    size_t bufSize = sizeof(buf);
    //Need to check the QoS level of the message to determine how the client should respond.
    //For QoS level 1, the client needs to send a PubAck message back to the server so it knows
    //it got the message.

    //DEBUG
    printf("%s%u\n", "Qos: ", QoS);

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

/**
 * Used to read in a publish message for a subscribed client.
 * @param subTopicID Used to check that this is a topic the client is subscribed and that the message received is one
 * //the client should be receiving.
 * @return An int
 */ 
int readPub(uint16_t subTopicID)
{
    //Used to read in a message.
    unsigned char buf[1600];
    size_t bufSize = sizeof(buf);

    //Used to hold flags and data of the Publish message
    unsigned char dup = 0;
    int qos = 0;
    unsigned char retained = 0;
    unsigned short pubMsgID = 0;
    MQTTSN_topicid pubTopic;
    unsigned char *pubData = 0;
    int dataLen = 0;

    if(MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_PUBLISH)
    {
        if(MQTTSNDeserialize_publish(&dup, &qos, &retained, &pubMsgID, &pubTopic, &pubData, &dataLen, buf, bufSize) == 1)
        {
            if(pubTopic.data.id == subTopicID)
            {
                printf("\n%s\n", pubData);
                return Q_NO_ERR;
            }
            else
            {
                return Q_ERR_SubTopic;
            }
            
        } 
        else
        {
            puts("Error with deserializing publish message.");
            return Q_ERR_Deserial;
        }
    }
    else
    {
        return Q_ERR_MsgType;
    }
        
}//End readPub

int readReg(Client_t *clientPtr, uint16_t *msgID)
{
    unsigned short topicID;
    unsigned short regMsgID;
    MQTTSNString topicName;
    unsigned char buf[1600];
    size_t bufSize = sizeof(buf);

    if(MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_REGISTER)
    {
        if(MQTTSNDeserialize_register(&topicID, &regMsgID, &topicName, buf, bufSize) == 1)
        {
            //Need this message ID for the client to respond with the RegAck message.
            *msgID = regMsgID;
            //This is the topicID for the wildcard topic the client subscribed to.
            clientPtr->topicID = topicID;
            return Q_NO_ERR;
        }
        else
        {
            return Q_ERR_Deserial;
        }
    }
    else
    {   
        return Q_ERR_MsgType;
    }
    
}//end readReg


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

        case Q_ERR_QoS:
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

        case Q_ERR_MsgReturnCode:
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

        case Q_WildCard:
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

        default:
            puts("Foreign return code");
            break;
    }
}
