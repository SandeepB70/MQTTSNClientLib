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
#define Q_ERR_SocketOpen 30
//indicate error for failure to receive WillTopicReq from server.
#define Q_ERR_WillTopReq 5
//Indicates that the Will Topic will be prompted for by server so we need to call WillTopic function.
//FIX THE NAME
#define Q_WillTopReq 51


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
//FIX THE NAME
#define Q_WillMsgReq 14
#define Q_ERR_WillMsgReq 15

//Publish.c
#define Q_ERR_TopicIdType 16
#define Q_ERR_PubAck 17
#define Q_ERR_MsgID 18
#define Q_ERR_MsgReturnCode 19
#define Q_ERR_MsgType 20
#define Q_ERR_Unknown 21
#define Q_ERR_PubRel 31
//Indicates client needs to resend a PubRel message.
#define Q_ERR_SendRel 32
//Indicates client needs to resend a Publish message.
#define Q_ERR_RePub 33

//Subscribe.c
#define Q_ERR_SubAck 22
//Indicates client subscribed to a topic using a wildcard character.
#define Q_WildCard 23

//PingReq.c
//Indicates that there are no messages for a client that is has woken up from sleep
#define Q_NoMsg 24
#define Q_ERR_WrongTopicID 25
#define Q_ERR_PubRec 26
#define Q_ERR_qosResponse 27
#define Q_PubMsgRead 28

//Util.c
#define Q_ERR_PubComp 29
#define Q_ERR_SubTopic 34
