//Builds and Sends a RegAck message for the client


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>


#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNPublish.h"
#include "ErrorCodes.h"
#include "RegAck.h"
#include "transport.h"
#include "StackTrace.h"

/**
 * Builds and sends out a RegAck message meant to be sent from the client. This should only be used when a client
 * reconnects with no CleanSession flag set or when it subscribes to a topic name that has wildcard characters. 
 * @param clientPtr The client who will be sending out the acknowledgment message
 * @param topicID The topicID that should have been sent by the server in its register message.
 * @param msgID The messsage ID that should have been sent by the server.
 * @return An int: Q_NO_ERR indicates success. Otherwise, Q_ERR_Serial, Q_ERR_Socket indicate errors.
 */

int regAck(Client_t *clientPtr, uint16_t topicID, uint16_t msgID, uint8_t msgReturnCode)
{

    int returnCode = Q_ERR_Unknown;

    //The return code we want to set for the RegAck message being sent. Since this is the client, 
    //it will acknowledge that it received and processed the register message with an "accepted" (0) return code.
    uint8_t regAckReturnCode = msgReturnCode;

    //The number of bytes needed in the buffer. Since this is a RegAck message, it will only be 7 bytes.
    size_t bufBytes = 7;

    //buffer to hold the message
    unsigned char buf[bufBytes];

    //The size of the buffer.
    size_t bufSize = sizeof(buf);

    FUNC_ENTRY;
    //The serialized length of the message
    size_t serialLength = 0;

    //Check if the return code provided is within the valid range.
    if(regAckReturnCode > 3){
        returnCode = Q_ERR_Rejected;
        goto exit;
    }

    //Serialize the message and then check the returnCode.
    returnCode = MQTTSNSerialize_regack(buf, bufSize, topicID, msgID, regAckReturnCode);

    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    //OLD CODE
    //TODO Ask Lawrence about this. Need to figure out a way to create a buffer large enough
    //to hold the publish message (if it has been sent). May have to also 
    //adjust transport_getdata() so that it receives a flag for recvfrom (MSG_PEEK) or
    //(MSG_WAITALL). 
    /**
     * Only read the three bytes of the message to check for a length. If the first octet is 0x01, the next two octets
     * contain the actual length of the message. Otherwise the first octet contains the entire length of the message. A new buffer
     * will have to be made to hold the publish message.
     */
    /**
    unsigned char buf2[3];
    size_t bufSize2 = sizeof(buf2);
    //Check the very first byte of any message that needs to be received. If recvfrom does not return greater than zero,
    //there is no message to be received.
    if( recvfrom(clientPtr->mySocket, buf2, bufSize2, MSG_PEEK) > 0)
    {
        //Value of the first byte of the message if the message has a three byte length field.
        uint8_t threeByteLen = 0x01;

        //The number of bytes needed to hold the new message.
        size_t bufBytes3 = 0;

        //Obtain the length of this new message.
        int msgLength = 0;

        //The next two bytes give us the message length.
        if(buf2[0] == threeByteLen)
        {
            //Obtain the length of this new message.
            msgLength = readInt(&buf2[1]);
        }
        //The message length is in the first byte of the message.
        else
        {
            msgLength = readChar(&buf2[0]);
        }
        

        if(msgLength > 0) 
        {
            bufBytes3 = (size_t) msgLength;
        }

        //A new buffer to hold the publish message if it has been sent by the server.
        unsigned char buf3[bufBytes3];

        //The size of the new buffer needed to hold the message.
        size_t bufSize3 = sizeof(buf3);

        int msgType = MQTSNPacket_read(buf3, bufSize3, transport_getdata);

        //Check if this is a publish message, 
        if(msgType == MQTTSN_PUBLISH)
        {
            MQTTSNFlags flags;
            uint16_t msgID = 0;
            MQTTSN_topicid topic;
            //Represents the length of the data in bytes within the message only.
            int dataSize = 0; 

            //Used to hold the data portion of the Publish message
            unsigned char *data;

            if(MQTTSNDeserialize_publish(&(flags.dup), &(flags.qos), &(flags.retain), &msgID, &topic, data, dataSize, buf3, bufSize3) == 1)
            {
                if(flags.qos == 1 || flags.qos == 2){
                    //Have to send back a PubAck
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
            printf("\n%s%d\n", "Actual Message Type: ", msgType);
            goto exit;
        }

    }//End outermost if
    */
    //If no publish message has been received, then there are no other messages to be expected from the server.
    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
