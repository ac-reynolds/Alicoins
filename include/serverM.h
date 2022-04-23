#include "globals.h"

#define BACKLOG 10

typedef struct Transaction {
  int transactionID;
  char sender[MAX_NAME_LENGTH];
  char receiver[MAX_NAME_LENGTH];
  int amt;
} transaction;

const int LISTENER_PORTS[3] = {CA2SMPort, CB2SMPort, SX2SMPort};
const int PORTS_TO_SERVERS[3] = {SM2SAPort, SM2SAPort, SM2SAPort};
const int LISTENER_SOCKET_TYPES[3] = {SOCK_STREAM, SOCK_STREAM, SOCK_DGRAM};

// Socket file descriptors
int socketParents [3];

int initializeSockets();
int acceptClientConnections();

// For UDP only
int requestBackendResponse(int port, void *req, int reqSize, void *res, int resSize);

// Application logic
int handleClientMessage(int sockfd, unsigned short port, clientResponse *res);
int doUserCheck(char *name, clientResponse *res);
