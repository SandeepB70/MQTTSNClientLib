/**
 * This header file stores all the error codes that can be returned by each of the
 * MQTTSN Client messages and testing files.
 */
 

//Connect.c
#define Q_NO_ERR 0
#define Q_ERR_Socket 1
#define Q_ERR_Connect 2
#define Q_ERR_Deserial 3
#define Q_ERR_Connack 4
//indicate error for failure to receive WillTopicReq from server.
#define Q_ERR_WTR 5
//Indicates that the Will Topic will be prompted for by server so we need to call WillTopic function.
#define Q_WTR 51

//connect_test.c does not need any extra error codes

//Disconnect.c
#define Q_ERR_Disconnect 6
#define Q_ERR_Ack 7

//disconnect_test.c
#define Q_ERR_CSocket 8
#define Q_ERR_DSocket 9

//WillTopic.c
#define Q_ERR_QoS 10
#define Q_ERR_Retain 11
#define Q_ERR_Serial 12
#define Q_ERR_StrCreate 13
//Indicates WillMsgReq has been sent by server so we need to call the WillMsg function.
#define Q_WMR 14
//Indicates error for failure to receive WillMsgReq from server.
#define Q_ERR_WMR 15

//will_test.c does not need any extra error codes.
