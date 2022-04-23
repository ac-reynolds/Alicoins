#include "serverM.h"

/* Messages */
void printBootUp() { 
  printf("The main server is up and running.\n");
}

void printUserCheck(char* usr, int port) {
  printf("The main server received input=%s from the client using TCP over port %d.\n", usr, port);
}

void printUserTransfer(char* sender, char* receiver, int amt, int port) {
  printf("The main server received from %s to transfer %d coins to %s using TCP over port %d.\n", sender, amt, receiver, port);
}

/* Initializes both sockets for incoming communication, but does not start accepting connections. */
int initializeSockets() {
  int status = 0;

  // Initialize once for each TCP socket to listen on
  for (int i = 0; i < 2; i++) {
    // Create parent sockets
    socketParents[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (socketParents[i] < 0) {
      perror("Could not create parent TCP socket.\n");
      return socketParents[i];
    }
  
    int optval = 1;
    status = setsockopt(socketParents[i], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (status) {
      perror("Could not set TCP socket option.\n");
      return status;
    }

    // Define addresses to be connected to by each client
    struct sockaddr_in addrs[2];
    addrs[i].sin_family = AF_INET;
    addrs[i].sin_port = htons(PORTS_TO_CLIENTS[i]);
    status = inet_aton(localhost, &addrs[i].sin_addr);
    if (!status) {
      perror("Could not convert host address for TCP socket.\n");
      return status;
    }

    // Bind each socket to the appropriate address
    status = bind(socketParents[i], (struct sockaddr *)&addrs[i], sizeof(addrs[i]));
    if (status) {
      perror("Could not bind TCP socket.\n");
      return status;
    }

    // Set each socket to passively listen
    status = listen(socketParents[i], BACKLOG);
    if (status) {
      perror("Could not set TCP socket to listening state.\n");
      return status;
    }
  }

}

int handleTCPMessage(int sockfd, unsigned short port) {
  int status = 0;
  
  clientRequest req;
  recv(sockfd, (clientRequest *)&req, sizeof(req), 0);
  switch (req.requestType) {
    case CLIENT_CHECK:
      printUserCheck(req.sender, port);
      break;
    case CLIENT_TRANSFER:
      printUserTransfer(req.sender, req.receiver, req.amt, port);
      break;
  }

  return 0;
}

/* Passively awaits connections, blocking until a connection is received. On
   such receipt, the connection is accepted and appropriate action is taken. */
int acceptConnections() {
  int status = 0;
  socklen_t incomingAddrSize;
  struct sockaddr incomingAddrs[2];
  int incomingSockets[2];

  incomingAddrSize = sizeof(struct sockaddr);
  while(1) {
    for (int i = 0; i < 2; i++) {
      incomingSockets[i] = accept(socketParents[i], &incomingAddrs[i], &incomingAddrSize);
      status = handleTCPMessage(incomingSockets[i], PORTS_TO_CLIENTS[i]);
      if (status) return status;
      close(incomingSockets[i]);
    }
  }
}

int main() {

  int status = 0;

  status = initializeSockets();
  if (status) return status;
  printBootUp();
  status = acceptConnections();
  if (status) return status;
  return 0;
}
