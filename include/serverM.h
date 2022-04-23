#include "globals.h"

#define BACKLOG 10

const int INDEX_TO_SOCKET_ID_OFFSET = 65;
const int PORTS_TO_CLIENTS[2] = {CA2SMPort, CB2SMPort};

// Socket file descriptors
int socketParents [2];
