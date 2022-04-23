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
  for (int i = 0; i < 3; i++) {
    // Create parent sockets
    socketParents[i] = socket(AF_INET, LISTENER_SOCKET_TYPES[i], 0);
    if (socketParents[i] < 0) {
      perror("Could not create parent socket.");
      return socketParents[i];
    }
  
    int optval = 1;
    status = setsockopt(socketParents[i], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (status) {
      perror("Could not set socket option.");
      return status;
    }

    // Define addresses to be connected to by each client
    struct sockaddr_in addrs[2];
    addrs[i].sin_family = AF_INET;
    addrs[i].sin_port = htons(LISTENER_PORTS[i]);
    status = inet_aton(localhost, &addrs[i].sin_addr);
    if (!status) {
      perror("Could not convert host address for socket.");
      return status;
    }

    // Bind each socket to the appropriate address
    status = bind(socketParents[i], (struct sockaddr *)&addrs[i], sizeof(addrs[i]));
    if (status) {
      perror("Could not bind socket.");
      return status;
    }

    // Set each socket to passively listen
    if (LISTENER_SOCKET_TYPES[i] == SOCK_STREAM) {
      status = listen(socketParents[i], BACKLOG);
      if (status) {
        perror("Could not set socket to listening state.");
        return status;
      }
    }
  }

  return status;
}

/* Fowards the request to the specified server. Records the response in the res field. */
int requestBackendResponse(int port, void *req, int reqSize, void *res, int resSize) {
  int status = 0;

  // Create socket to send UDP datagram
  int socketOut = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketOut < 0) {
    perror("Could not create socket.");
    return socketOut;
  }

  // Convert serverM host
  struct sockaddr_in servAddr;
  servAddr.sin_port = htons(port);
  status = inet_aton(localhost, &servAddr.sin_addr);
  if (!status) {
    perror("Could not convert host address.");
    return status;
  }

  // Send datagram
  status = sendto(socketOut, req, reqSize, 0, 
    (struct sockaddr *)&servAddr, sizeof(struct sockaddr_in));
  if (status < 0) {
    perror("Error sending datagram");
    return status;
  }

  // Await response, filling out data based on type of request sent
  struct sockaddr incomingAddr;
  socklen_t incomingAddrSize = sizeof(incomingAddr);
  printf("BLOCKING...\n");
  status = recvfrom(socketParents[2], res, resSize, 0, &incomingAddr, &incomingAddrSize);
  printf("RECEIVED %s\n", (char *)res);

  if (status < 0) {
    perror("Error receiving UDP message");
    return status;
  }  

  status = close(socketOut);
  if (status) {
    perror("Error closing socket.");
    return status;
  } 
}

int doUserCheck(char* name, clientResponse *res) {
  int status = 0;

  char log[MAX_NUM_TRANSACTIONS];

  serverRequest req;
  req.requestType = SERVER_CHECK;
  req.terminalOperation = TRUE;
  // initialize 3000 entry table

  int confirmed;
  int i = 0;
    status = requestBackendResponse(PORTS_TO_SERVERS[i], (void *)&req, sizeof(req), (void *)log, 100);
    printf("messag rec'd is %s.\n", log);

  // socket to backend

  // fill in entries

  // compute
}

/* Interprets the client message. Sets res to the response to be sent back to the client. */
int handleClientMessage(int sockfd, unsigned short port, clientResponse *res) {
  int status = 0;
  
  clientRequest req;
  serverRequest fwdReq;
  recv(sockfd, (clientRequest *)&req, sizeof(req), 0);
  switch (req.requestType) {
    case CLIENT_CHECK:
      printUserCheck(req.sender, port);
      doUserCheck(req.sender, res);
      break;
    case CLIENT_TRANSFER:
      printUserTransfer(req.sender, req.receiver, req.amt, port);
      break;
  }

  return status;
}

/* Passively awaits client connections, blocking until a connection is received. On
   such receipt, the connection is accepted and appropriate action is taken. */
int acceptClientConnections() {
  int status = 0;
  socklen_t incomingAddrSize;
  struct sockaddr incomingAddrs[2];
  int incomingSockets[2];
  incomingAddrSize = sizeof(struct sockaddr);
  clientResponse res;

  // Loop forever, accepting incoming requests from each client
  while(1) {
    for (int i = 0; i < 2; i++) {
      incomingSockets[i] = accept(socketParents[i], &incomingAddrs[i], &incomingAddrSize);
      status = handleClientMessage(incomingSockets[i], LISTENER_PORTS[i], &res);
      if (status) return status;
      close(incomingSockets[i]);
    }
  }
}

/* Main driver */
int main() {
  int status = 0;

  status = initializeSockets();
  if (status) return status;

  printBootUp();

  status = acceptClientConnections();
  if (status) return status;

  return status;
}
