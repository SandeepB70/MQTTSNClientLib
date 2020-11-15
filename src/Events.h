/**
 * Defines the enumerations for events and the Client_Event structure 
 * which basically indicates what state the client will be transitioning to
 * when an event happens, such as a certain type of message being sent.
 */ 

#include <stdint.h>

enum Q_MQTTSN_EVENT {
    Q_CONNECTING, Q_CONNECTED, Q_WILL_TOP_REQ, Q_WILL_MSG_REQ,
    Q_REGISTERING, Q_SLEEP, Q_SUBSCRIBING, Q_DISCONNECTING,
    Q_DISCONNECTED, Q_WILL_TOP_UPD, Q_WILL_MSG_UPD, Q_UNSUBSCRIBE, 
    Q_CLIENT_PING, Q_SERVER_PING, Q_PUBLISH, Q_PUB_QOS2, Q_RCV_QOS1, Q_RCV_QOS2    
};

typedef struct {
    enum Q_MQTTSN_EVENT eventID;
    //Following three variables are used for comparison purposes to check that
    //the corresponding parameters of the message sent by the client match 
    //with those received by the server (for ex. the msgIDs must match).
    uint16_t msgID;
    uint16_t topicID;
    unsigned char qos;
    //Used to store the Keep Alive timer in seconds for the client.
    uint16_t duration;
    //Used to keep track of the message id for messages sent from client.
    uint16_t send_msgID;
    Client_t *client;
} Client_Event_t;

