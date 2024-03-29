//Percentage Contribution: Sandeep Bindra (100%)
//Builds and sends the WillTopicUpd message.

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "ErrorCodes.h"
#include "Client_t.h"
#include "StackTrace.h"

/**
 * Builds and sends a WillTopicUpd message for the client.
 * @param clientPtr The client sending out the WillTopicUpd message.
 * @param flags Represents the flags portion of the message. Only the QoS and retain flag are relevant for this message.
 * @param willTopic Holds the new WillTopic for the client.
 * @return An int: Q_NO_ERR indicates that the message was sent successfully and accepted by the Server/GW.
 * Otherwise, Q_ERR_Unknown, Q_ERR_Serial, or Q_ERR_Socket indicate an error.
 */
int WillTopicUpd(Client_t *clientPtr, const MQTTSNFlags *flags, const MQTTSNString *willTopic) 
{
    int returnCode = Q_ERR_Unknown;

    //Number of bytes in the buffer that holds the message
    size_t bufBytes = 0;

    //Size of the buffer
    size_t bufSize = 0

    //Length of the serialized message.
    size_t serialLength = 0;

    FUNC_ENTRY;
    //TODO, The way the lower code is written, packet size will always be at least 3 bytes so
    //the WillTopicUpd message can never be 2 bytes for an empty message to delete Will data. 
    bufBytes = MQTTSNPacket_len(MQTTSNstrlen(willTopic));

    //The buffer that will hold the WillTopicUpd message.
    unsigned char buf[bufBytes];

    //Obtain the size of the buffer.
    bufSize = sizeof(buf);

    //Serialize the message.
    returnCode = MQTTSNSerialize_willtopicupd(buf, bufSize, flags->QoS, flags->retain, *willTopic);

    //Make sure the serialization was successful.
    if(returnCode > 0){
        serialLength = returnCode;
    }
    else{
        returnCode = Q_ERR_Serial;
        goto exit;
    }

    //Send the message
    ssize_t returnCode2 = transport_sendPacketBuffer(clientPtr->host, clientPtr->destinationPort, buf, serialLength);

    //Ensure that the message was successfully sent
    if(returnCode2 != 0){
        returnCode = Q_ERR_Socket;
        goto exit;
    }

    returnCode = Q_NO_ERR;

exit:
    FUNC_EXIT_RC(returnCode);
    return returnCode;
}
