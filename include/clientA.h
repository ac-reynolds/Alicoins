#include "globals.h"

const char clientID = 'A';
const int assignedPort = CA2SMPort;

// fd for outgoing socket
int sockfd;

// Layer 4 utility
int connectToServer();
int requestServerResponse(clientRequest *req, clientResponse *res);
int disconnectFromServer();

// Application logic
int doUserCheck(char *usr);
int doUserTransfer(char *sender, char *receiver, int amt);
