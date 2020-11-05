//Header file for Util.c

int MQTTSNStrCreate(MQTTSNString *strContainer, char *string); //prototype
int qosResponse(Client_t *clientPtr, const uint8_t QoS, const uint16_t msgID, const uint16_t topicID); //prototype
void returnCodeHandler(int returnCode); //prototype
int readPub(uint16_t subTopicID); //prototype
int readReg(Client_t *clientPtr, uint16_t *msgID); //prototype


