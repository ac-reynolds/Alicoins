#include "globals.h"

const char CLIENT_ID = 'A';
const int ASSIGNED_PORT = CA2SMPort;

// fd for outgoing socket
int sockfd;

// Layer 4 utility
int connectToServer();
int requestServerResponse(clientRequest *req, clientResponse *res);
int disconnectFromServer();

// Application logic
int doUserCheck(char *usr);
int doUserTransfer(char *sender, char *receiver, int amt);
int doTXLIST();
