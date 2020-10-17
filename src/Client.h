//Represents the Client, which will be composed of the destination port it will send MQTTSN messages to, 
//the destination IP address (host), the id of the client, and the socket the client will be using to communicate.

typedef struct {
    int destinationPort;
    char *host;
    int mySocket;
    char *clientID;
} Client;
