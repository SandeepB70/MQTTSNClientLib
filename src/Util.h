//Header file for Util.c

int MQTTSNStrCreate(MQTTSNString *strContainer, char *string); //prototype
//int qosResponse(Client_t *clientPtr, const uint8_t QoS, const uint16_t msgID, const uint16_t topicID); //prototype
void returnCodeHandler(int returnCode); //prototype
int readConnack(unsigned char *buf, size_t bufSize); //prototype
int readWillTopResp(unsigned char *buf, size_t bufSize); //prototype
int readWillMsgResp(unsigned char *buf, size_t bufSize); //prototype
int readUnsubAck(unsigned char *buf, size_t bufSize, Client_Event_t *event); //prototype
int readSubAck(unsigned char *buf, size_t bufSize, Client_Event_t *event); //prototype
int readPub(unsigned char *buf, size_t bufSize, Client_Event_t *event); //prototype
int readReg(unsigned char *buf, size_t bufSize, Client_Event_t *event); //prototype
int readRegAck(unsigned char *buf, size_t bufSize, Client_Event_t *event); //prototype
int readPubAck(unsigned char *buf, size_t bufSize, Client_Event_t *event); //prototype
int readRecRelComp(unsigned char *buf, size_t bufSize, Client_Event_t *event, unsigned char msgType); //prototype
int msgReceived(int clientSock); //prototype
int readMsg(Client_Event_t *event); //prototype


