#include "serverA.h"

/* Messages */
void printBootUp() {
  printf("The Server%c is up and running using UDP on port %d.\n", SERVER_ID, PORT); 
}

void printRequest() {
  printf("The Server%c received a request from the Main Server.\n", SERVER_ID);
}

void printResponse() {
  printf("The Server%c finished sending the response to the Main Server.\n", SERVER_ID);
}

/* Initializes socket to listen on, as well as socket to respond on. */
int initializeSockets() {
  int status = 0;

  // Create parent socket
  sockets[LISTENER] = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockets[LISTENER] < 0) {
   perror("Could not create UDP socket.\n");
   return sockets[LISTENER];
  }

  // Set socket addr to be reused (no zombies!)
  int optval = 1;
  status = setsockopt(sockets[LISTENER], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (status) {
    perror("Could not set UDP socket option.\n");
  }

  // Define address to be connected to
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  status = inet_aton(localhost, &addr.sin_addr);
  if (!status) {
    perror("Could not convert host address for UDP socket.\n");
    return status;
  }

  // Bind listener socket
  status = bind(sockets[LISTENER], (struct sockaddr *)&addr, sizeof(addr));
  if (status) {
    perror("Could not bind UDP socket.\n");
    return status;
  }

  return status;
}

int sendResponse(void *res, int resSize) {
  int status = 0;

  // Create socket to send UDP datagram
  int socketOut = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketOut < 0) {
    perror("Could not creat socket");
    return socketOut;
  }

  // Convert host address
  struct sockaddr_in servAddr;
  servAddr.sin_port = htons(SX2SMPort);
  status = inet_aton(localhost, &servAddr.sin_addr);
  if (!status) {
    perror("Could not convert host address");
    return status;
  }

  // Send datagram
  status = sendto(socketOut, res, resSize, 0,
    (struct sockaddr *)&servAddr, sizeof(struct sockaddr_in));
  if (status < 0) {
    perror("Error sending datagram");
    return status;
  }

  // Close socket
  status = close(socketOut);
  if (status) {
    perror("Error closing socket");
    return status;
  }
}

int handleResponse(serverRequest* req) {
  int status = 0;
  char word[20] = "go to sleep\n";
  if (req->terminalOperation) {
    printRequest();
    status = sendResponse((void *)word, sizeof(word));
    if (status) return status;
    printResponse();
  } else printf("processing nonterminal operation.\n");
  
  return status;
}

int acceptConnections() {
  int status = 0;
  socklen_t incomingAddrSize;
  struct sockaddr incomingAddr;
  incomingAddrSize = sizeof(incomingAddr);
  serverRequest req;

  // Loop forever, accepting incoming request from main server
  while(1) {
    status = recvfrom(sockets[LISTENER], &req, sizeof(req), 0, &incomingAddr, &incomingAddrSize);
    if (status < 0) return status;
    status = handleResponse(&req);
    if (status) return status;
  }
  return status;
}

/* Main driver */
int main() {
  int status = 0;

  status = initializeSockets();
  if (status) return status;

  printBootUp();

  status = acceptConnections();
  if (status) return status;

  return status;
}
