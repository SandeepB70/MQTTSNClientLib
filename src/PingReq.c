//Builds and sends out a PingReq message. 


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "Client_t.h"
#include "MQTTSNConnect.h"
#include "MQTTSNPacket.h"
#include "ErrorCodes.h"
#include "PingReq.h"
#include "PubAck.h"
#include "StackTrace.h"
#include "transport.h"
#include "Util.h"

/**
 * Following function is to build and send a simple PingReq message. 
 * If a clientID is included (the length of the clientID string is not 0), then this is a sleeping client
 * that will go through the procedure of prompting the Server/GW for any buffered messages.
 * @param clientPtr The client who will be sending out the PingReq message.
 * @param clientID The client ID of sending client. This can be empty (in the form of a clientID with length of 0),
 * or can contain a valid client ID, which will cause the server/GW to check if it has any buffered message for this client.
 * @return An int: Q_NO_ERR (0) for success of a simple ping. Q_PubMsgRead(29) indicates Publish messages were 
 * received by a sleeping client and Q_NoMsg (25) indicates there were no messages for the sleeping client.
 * Otherwise, Q_ERR_Unknown, Q_ERR_Serial, Q_ERR_Socket, Q_ERR_MsgType, Q_ERR_qosResponse, Q_ERR_WrongTopicID, Q_ERR_PubAck,
 * or Q_ERR_Deserial all indicate an error.
 */

int pingReq(Client_t *clientPtr, MQTTSNString *clientID)
{
    int returnCode = Q_ERR_Unknown;

    //The number of bytes needed by the buffer.
    size_t bufBytes = 0;

    //The size of the buffer.
    size_t bufSize = 0;

    //Serialized length of the message.
    size_t serialLength = 0;

    //Checking if we want to specify the client ID for this PingReq message.
    //If we are not including a clientID, the buffer will only need to hold two bytes of data.
    if(clientID->lenstring.len == 0){
        bufBytes = 2;
    }
    else{
        //Add 1 for the MsgType portion of the message.
        bufBytes = MQTTSNPacket_len(MQTTSNstrlen(*clientID) + 1);
    }

    //The buffer to hold the message.
    unsigned char buf[bufBytes];

    bufSize = sizeof(buf);

    //Serialize the message
    returnCode = MQTTSNSerialize_pingreq(buf, bufSize, *clientID);
    
    //Check if serialization was successful and if it was, assign the serialized length of the message.
    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if (returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    //Create a new buffer to hold a large number of bytes to read in whatever message the server
    //sends back to the client.
    unsigned char buf2[1600];

    //The size of the new buffer
    size_t bufSize2 = sizeof(buf2);

    /**
     * If the client sent a clientID with the PingReq, then it is now awake from its asleep state
     * and is prompting the server for any messages that are being held for it. We will then have to check
     * for Publish messages. If there are no messages for the client, the server will respond immediately with
     * a PingResp message. The server will also respond with a PingResp message if the clientID was not included
     * with this PingReq. 
     */ 

    //Checking if the clientID was not included with this message, which means we only expect a PingReq message back.
    if(clientID->lenstring.len == 0)
    {
        if(MQTTSNPacket_read(buf2, bufSize2, transport_getdata) == MQTTSN_PINGRESP)
        {
            returnCode = Q_NO_ERR;
            goto exit;
        }
        else
        {
            returnCode = Q_ERR_MsgType;
            goto exit;
        }
        
    }
    //A clientID was included in this PingReq message so we need to check if the server has messages for the client
    //which will be Publish messages. Otherwise, the server will be sending back a PingResp message for this client.
    else 
    {
        //Flag for the loop.
        bool loopFlag = true;

        //Lets the client know if Publish messages were ever received by the server
        //so the returnCode can return the correct status code.
        bool pubMsgRead = false;

        //This variable will be used to indicate whether or not
        //the topicID sent by the server matches with the topicID the Client is subscribed to.
        bool topicIDMatch = false;

        while(loopFlag)
        {
            if(MQTTSNPacket_read(buf2, bufSize2, transport_getdata) == MQTTSN_PUBLISH)
            {   
                //The following variables are needed to extract information from the Publish message
                //when it is deserialized.
                unsigned char dup = 0;
                int qos = 0;
                unsigned char retained = 0;
                unsigned short pubMsgID = 0;
                MQTTSN_topicid pubTopic;
                unsigned char *pubData = 0;
                int dataLen = 0;
                //Used for error code status that indicates a publish message had been read.
                pubMsgRead = true;
                
                if(MQTTSNDeserialize_publish(&dup, &qos, &retained, &pubMsgID, &pubTopic, &pubData, &dataLen, buf2, bufSize2) == 1)
                {
                    //Check if this is a topicID that the client is subscribed to. Need to check the topic type
                    //to ensure the proper members of the MQTTSN_topicid struct are being compared to the client's topic ID.
                    if(pubTopic.type == MQTTSN_TOPIC_TYPE_NORMAL || pubTopic.type == MQTTSN_TOPIC_TYPE_PREDEFINED)
                    {
                        //The topicID of this Publish message matches with what the client is subscribed to.
                        if(clientPtr->topicID == pubTopic.data.id){
                            topicIDMatch = true;
                        }
                    }
                    //The topicID is going to be contained within the short_name member of the MQTTSN_topicid struct
                    else
                    {
                        //Will be used to hold the number that represents the short topic name.
                        uint16_t shortName = 0;
                        //Copy the two bytes of memory that represent the short topic name.
                        memcpy(&shortName, pubTopic.data.short_name, sizeof(short));

                        if(clientPtr->topicID == shortName){
                            topicIDMatch = true;
                        }
                    }
                    
                    //If this was a topicID the client subscribed to, print out the message.
                    if(topicIDMatch)
                    {
                        uint8_t convQos = (uint8_t)qos;
                        printf("%s\n", pubData);
                        //Check the QoS level and make sure the Client responds appropriately
                        if(qosResponse(clientPtr, convQos, pubMsgID, pubTopic.data.id) != Q_NO_ERR)
                        {
                            returnCode = Q_ERR_qosResponse;
                            loopFlag = false;
                            goto exit;
                        }
                    }
                    //Otherwise, must send a PubAck message with a reject return code.
                    else
                    {
                        //The rejection return code
                        uint8_t msgRtrnCode = 2;

                        //Must send a PubAck message to reject the topicID since the client didn't subscribe to it.
                        if(pubAck(clientPtr, pubTopic.data.id, pubMsgID, msgRtrnCode) == Q_NO_ERR)
                        {
                            returnCode = Q_ERR_WrongTopicID;
                            loopFlag = false;
                            goto exit;
                        }
                        //The PubAck message failed to send.
                        else
                        {
                            returnCode = Q_ERR_PubAck;
                            loopFlag = false;
                            goto exit;
                        }
                    }
                }
                //Failed to deserialize the message.
                else
                {
                    returnCode = Q_ERR_Deserial;
                    loopFlag = false;
                    goto exit;
                }
                
            }//End if statement checking for Publish message.

            //A PingResp message is only 2 bytes long so we know the second byte contains the MsgType.
            else if(buf2[1] == MQTTSN_PINGRESP)
            {
                //Determine whether or not a Publish message was read, otherwise the server had no messages for the client.
                if(pubMsgRead == true){
                    returnCode = Q_PubMsgRead;
                    loopFlag = false;
                    goto exit;
                }
                else{
                    returnCode = Q_NoMsg;
                    loopFlag = false;
                    goto exit;
                }  
            }
            //This is not the message we are expecting.
            else
            {
                returnCode = Q_ERR_MsgType;
                loopFlag = false;
                goto exit;
            }
        }//End while loop
    }//end else statement

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
