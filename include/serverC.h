#include "globals.h"

const char SERVER_ID = 'C';
const int PORT = SM2SCPort;
const int BACKLOG = 5;
const char BLOCK_FILE_NAME[] = "src/block3.txt";

int sockets[2];
struct sockaddr_in mainServAddr;

// Layer 4 utility
int initializeServerSocket();
int receiveRequests();
int sendResponse(serverResponse *res);

// Application logic
int handleRequest(serverRequest *req);

