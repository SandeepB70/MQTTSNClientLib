//Represents the Client, which will be composed of the destination port it will send MQTTSN messages to, 
//the destination IP address (host), the id of the client, and the socket the client will be using to communicate.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int destinationPort;
    char *host;
    int mySocket;
    char *clientID;
    //Used to hold the topicIDs for all subscribed topics, excluding wildcards.
    uint16_t sub_topicID[10];
    //Contains the number of topics subscribed to.
    size_t subscribe_Num;

    //Used to hold the topicIDs for all topics the can publish to.
    uint16_t pub_topicID[10];
    //Contains the number of topics the client can publish to.
    size_t publish_Num;

    //Indicates if the client has any wildcard subscriptions.
    bool wildcard_Sub;
    //The number of wildcard subscriptions the client has.
    size_t sub_Wild_Num;
    //Used to hold the topicID (obtained by a register message from the server)
    //for any wildcard topics the client subscribed to.
    uint16_t wild_topicID;
} Client_t;
