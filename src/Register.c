//Build and send a Register message for a client.

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNPublish.h"
#include "ErrorCodes.h"
#include "Register.h"
#include "transport.h"
#include "StackTrace.h"

/**
 * Builds and sends out a register message for the provided Client.
 * @param clientPtr The client that will be sending out a message.
 * @param msgID Used to identify this particular message and match it with the regack.
 * @param topicname The name of the topic the client is trying to register with the server.
 * @param regTopicID Used to store the returned topicID from the server if the TopicName is registered successfully.
 * @return An int: Q_NO_ERR (0) is a success. Otherwise, Q_ERR_Unknown (22), Q_ERR_Serial (12), Q_ERR_Socket (1), 
 * Q_ERR_Deserial (3), Q_ERR_Ack(7), Q_ERR_MsgReturnCode (19), or Q_ERR_MsgID (18) indicate an error. 
 */

int reg(Client_t *clientPtr, uint16_t msgID, MQTTSNString *topicname, uint16_t *regTopicID)
{   
    int returnCode = Q_ERR_Unknown;

    //Since this is being sent by the client, it must be 0.
    //This variable will also be used later to store the returned topicID 
    //from the server if registration is successful.
    uint16_t topicID = 0;

    //Number of bytes needed in the buffer.
    size_t bufBytes = 0;

    //Size of the entire buffer the message is written into.
    size_t bufSize = 0;

    //Size of the serialized message in bytes.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //Get the length of the topic name in order to determine how many bytes the buffer needs to hold.
    size_t topicNameLen = (topicname->cstring) ? strlen(topicname->cstring) : topicname->lenstring.len;

    bufBytes = MQTTSNPacket_len(MQTTSNSerialize_registerLength(topicNameLen));

    //buffer that will hold the Register message
    unsigned char buf[bufBytes];
    bufSize = sizeof(buf);

    returnCode = MQTTSNSerialize_register(buf, bufSize, topicID, msgID, topicname);
    
    //Check if serialization of the message was successful, in which case it returns the length of the serialized message.
    if(returnCode > 0){
        serialLength = (size_t) returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    //Check that the message was successfully sent to the server.
    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    //Check that a RegAck message was received and check if the return code is "accepted".
    if(MQTTSNPacket_read(buf, bufSize, transport_getdata) == MQTTSN_REGACK)
    {
        uint8_t returnCode3 = 10;
        uint16_t msgID2 = 0;

        //TODO Make the program wait Twait (5 minutes recommended) if return code congested is received. 
        //Ask Lawrence if sleep() is ok for this.
        //Deserialize the message and make sure the process was successful
        if(MQTTSNDeserialize_regack(&topicID, &msgID2, &returnCode3, buf, bufBytes) == 1)
        {
            //Make sure this is the corressponding RegAck for the Register message that was sent.
            if(msgID2 == msgID)
            {
                //Check the return code of the message.
                if(returnCode3 == MQTTSN_RC_ACCEPTED)
                {
                    //Store the registered topicID for the client to use when publishing messages.
                    *regTopicID = topicID;

                    returnCode = Q_NO_ERR;
                    goto exit;
                }
                else
                {
                    printf("%s%d\n", "Return Code: ", returnCode3);
                    returnCode = Q_ERR_MsgReturnCode;
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

    }//end outer if

    else
    {
        returnCode = Q_ERR_Ack;
        goto exit;
    }
    

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
