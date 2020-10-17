//Testing for WillMsg.

#include <stdlib.h>
#include <stdint.h>

#include "Client.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "Disconnect.h"
#include "WillTopic.h"
#include "ErrorCodes.h"
#include "WillMsg.h"

int main(void)
{

    int returnCode;
    //Represents the value of the Keep Alive timer in the duration portion of the connect message.
    uint16_t keepAlive = 20;
    //Represents the will flag for the connect message.
    uint8_t willFlag = 1;
    //Represents the clean session flag of the connect message.
    uint8_t clnSession = 1;

    Client testClient;
    testClient.destinationPort = 60885;
    testClient.host = "50.255.7.18";
    testClient.clientID = "SandeepTest";

    returnCode = connect(&testClient, keepAlive, willFlag, clnSession);

    switch (returnCode) {
        //We have to send a WillTopic message to the server.    
        case Q_WTR:
            break;

        case Q_NO_ERR:
            puts("Success!!");
            break;

        case Q_ERR_Socket:
            puts("Socket Error");
            break;

        case Q_ERR_Connect:
            puts("Connect Error");
            break;

        case Q_ERR_Deserial:
            puts("Deserial Error");
            break;
        
        case Q_ERR_Connack:
            puts("Connack Error");
            break;

        default:
            puts("Unknown Error");
            break;
    }//End switch

    //This code gets executed if the Server has sent a WillTopicReq.
    if(returnCode == Q_WTR){

        uint8_t willQoS = 0b00;
        uint8_t willRetain = 0;
        MQTTSNString topic;
        //The will topic name.
        char *topicString = "TestingWillTopicAndMessage";

        //Create the MQTTSNString for the will topic.
        returnCode = MQTTSNStrCreate(&topic, topicString);

        //Do nothing if there is no error.
        if(returnCode == Q_NO_ERR){}
        else if(returnCode == Q_ERR_StrCreate){
            puts("Topic String Error");
            goto exit;
        }
        else{
            puts("Unknow error with MQTTSNStrCreate function");
            goto exit;
        }

        returnCode = willTopic(&testClient, willQoS, willRetain, topic);

        //Check the returnCode from willTopic function;
        switch (returnCode) {
            case Q_WMR:
                //We have to send a WillMsg to the server.
                break;

            case Q_ERR_QoS:
                puts("QoS error");
                break;

            case Q_ERR_Retain:
                puts("Socket Error");
                break;

            case Q_ERR_Serial:
                puts("Connect Error");
                break;

            case Q_ERR_Socket:
                puts("Deserial Error");
                break;

            default:
                puts("Unknown Error");
                break;
        }//End switch

    }//End if(Q_WTR)

    if(returnCode == Q_WMR)
    {
        MQTTSNString message;
        //The message to be placed in the WillMsg.
        char *messageString = "Message is a success.";
        returnCode = MQTTSNStrCreate(&message, messageString);

        //If the MQTTSNString has been successfully created, do nothing.
        if(returnCode == Q_NO_ERR){}

        else if(returnCode == Q_ERR_StrCreate){
            puts("Will Message String Error");
            goto exit;
        }

        else{
            puts("Unknown Error with MQTTSNStrCreate function.");
            goto exit;
        }

        returnCode = WillMsg(&testClient, message);

        switch(returnCode)
        {
            case Q_NO_ERR:
                puts("Success!!");
                break;

            case Q_ERR_Serial:
                puts("Serial Error");
                break;

            case Q_ERR_Socket:
                puts("Socket Error");
                break;

            case Q_ERR_Connack:
                puts("Connack Error");
                break;

            default:
                puts("Unknown error with WillMsg");
                break;
        }//end switch

    }//End if(Q_WMR)
    

	transport_close();
    
exit:
    return returnCode;
}


