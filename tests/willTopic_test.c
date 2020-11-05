//Testing for WillTopic.

#include <stdlib.h>
#include <stdint.h>

#include "Client_t.h"
#include "MQTTSNPacket.h"
#include "MQTTSNConnect.h"
#include "transport.h"
#include "Connect.h"
#include "Disconnect.h"
#include "WillTopic.h"
#include "ErrorCodes.h"
#include "Util.h"

int main(void)
{

    int returnCode;
    //Represents the value of the Keep Alive timer in the duration portion of the connect message.
    uint16_t keepAlive = 20;
    //Represents the will flag for the connect message.
    uint8_t willFlag = 1;
    //Represents the clean session flag of the connect message.
    uint8_t clnSession = 1;

    Client_t testClient;
    testClient.destinationPort = 60885;
    //The IP address of the server/gateway we want to connect to.
    testClient.host = "50.255.7.18";
    //ID of this client.
    testClient.clientID = "SandeepTest";

    returnCode = connect(&testClient, keepAlive, willFlag, clnSession);

    //Check the value of returnCode to ensure there were no errors.
    switch (returnCode) {
        //We have to send a WillTopic message to the server.    
        case Q_WillTopReq:
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
    if(returnCode == Q_WillTopReq){

        //Value of QoS flag.
        uint8_t willQoS = 0b00;
        //Value of the retain flag.
        uint8_t willRetain = 0;
        MQTTSNString topic;
        char *topicString = "TestingWillTopic";

        //Create the MQTTSNString for the will topic.
        returnCode = MQTTSNStrCreate(&topic, topicString);

        //Do nothing if there is no error.
        if(returnCode == Q_NO_ERR){}
        else if(returnCode == Q_ERR_StrCreate){
            puts("Topic String Error");
        }
        else{
            puts("Unknow error with MQTTSNStrCreate function");
        }

        returnCode = willTopic(&testClient, willQoS, willRetain, topic);

        //Check the returnCode from willTopic function;
        switch (returnCode) {
            case Q_WillMsgReq:
                puts("Success!!");
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

    }//End if(Q_WillTopReq)

	transport_close();

    return returnCode;
}


