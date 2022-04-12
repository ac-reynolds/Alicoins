#include "globals.h"
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

  // Create parent sockets
  socketParentA = socket(AF_INET, SOCK_STREAM, 0);
  if (socketParentA < 0) {
    perror("Could not create parent socket A.\n");
    return socketParentA;
  }

  // Define addresses to be connected to by each client
  struct sockaddr_in addrA;
  addrA.sin_family = AF_INET;
  addrA.sin_port = CA2SMPort;
  status = inet_aton(localhost, &addrA.sin_addr);
  if (!status) {
    perror("Could not convert host address for parent socket A.\n");
    return status;
  }

  // Bind each socket to the appropriate address
  status = bind(socketParentA, (struct sockaddr *)&addrA, sizeof(addrA));
  if (status) {
    perror("Could not bind parent socket A.\n");
    return status;
  }

  // Set each socket to passively listen
  status = listen(socketParentA, BACKLOG);
  if (status) {
    perror("Could not set parent socket to listening state.\n");
    return status;
  }

}

int acceptConnections() {
  struct sockaddr incomingAddrA;
  int incomingSocketA;
}

int main() {

  int status = 0;

  initializeSockets();
  printBootUp();
  acceptConnections();
  return 0;
}
