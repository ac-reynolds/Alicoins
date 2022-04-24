#include "globals.h"

#define BACKLOG 10

typedef struct Transaction {
  int transactionID;
  char sender[MAX_NAME_LENGTH];
  char receiver[MAX_NAME_LENGTH];
  int amt;
} transaction;

const int LISTENER_PORTS[3] = {CA2SMPort, CB2SMPort, SX2SMPort};
const int PORTS_TO_SERVERS[3] = {SM2SAPort, SM2SBPort, SM2SCPort};
const int LISTENER_SOCKET_TYPES[3] = {SOCK_STREAM, SOCK_STREAM, SOCK_DGRAM};

// Socket file descriptors
int socketParents [3];

// Application logic
int handleClientMessage(int sockfd, unsigned short port, clientResponse *res);
int doUserCheck(char *name, clientResponse *res);
void parseTransaction(char *str, transaction *entry);
int requestBackendResponse(int port, serverRequest *req, transaction *res);

// Layer 4 utility
int initializeServerSockets();
int acceptClientConnections();
