/**
 * Percentage Contribution: Sandeep Bindra (100%)
 * Defines a struct that holds information the Client needs. This includes the port being used to communicate with the 
 * Gateway(destinationPort), the destination IP address (host), the id of the client, and the socket the client will be using 
 * to communicate. The topic IDs of subscribed and registered topics will be stored as well, including their number. 
 * The client will only be allowed to subscribe and publish to 10 topicIDs, excluding topics with a wildcard character. 
 * Topics with a wildcard character will be stored separately and can be replaced easily since the topicID for wildcard topics 
 * can change when a new series of publish messages comes in.
 */ 

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    //The port the Gateway is using to communicate.
    int destinationPort;
    //The IP address of the gateway
    char *host;
    //Socket the client will use to communicate with the gateway
    int mySocket;
    //ID used to identify the client to the server.
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
