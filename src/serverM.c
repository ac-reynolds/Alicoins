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

/* For debugging/utility */
void printTransaction(transaction *t) {
  printf("TX%d: %s -> %s (amt = %d)\n", t->transactionID, t->sender, t->receiver, t->amt);
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
      status = doUserCheck(req.sender, res);
      if (status) return status;
      status = send(sockfd, res, sizeof(clientResponse), 0);
      if (status) return status;
      break;
    case CLIENT_TRANSFER:
      printUserTransfer(req.sender, req.receiver, req.amt, port);
      break;
  }

  return status;
}

/* Perform the check functionality */
int doUserCheck(char* name, clientResponse *res) {
  int status = 0;

  // initialize transaction table
  transaction log[MAX_NUM_TRANSACTIONS];

  // Set up request
  serverRequest req;
  req.requestType = SERVER_CHECK;
  req.terminalOperation = TRUE;

  // Fill in entries into table based on backend data
  int tableSize = 0;
  for (int i = 0; i < 1; i++) {
    status = requestBackendResponse(PORTS_TO_SERVERS[i], &req, log + tableSize);
    if (status < 0) return status;
    tableSize += status;
  }

  printf("parsed %d entries\n", tableSize);

  // Calculate funds associated with the passed user  
  int userFunds = INITIAL_ACCOUNT_VALUE; 
  for (int i = 0; i < tableSize; i++) {
    if (!strcmp(name, (log + i)->sender)) {
      userFunds -= (log + i)->amt;
      printTransaction(log + i);
    } 
    if (!strcmp(name, (log + i)->receiver)) {
      userFunds += (log + i)->amt;
      printTransaction(log + i);
    }
  }

  // Fill in response fields
  res->responseType = CLIENT_CHECK;
  res->result = userFunds;

  return 0;
}


/* Initializes both sockets for incoming communication, but does not start accepting connections. */
int initializeServerSockets() {
  int status = 0;

  // Initialize once for each server socket to listen on
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

/* Fowards the request to the specified server. Records the response in the res field. */
int requestBackendResponse(int port, serverRequest *req, transaction *log) {
  int status = 0;

  // Create socket to send UDP datagram
  int socketOut = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketOut < 0) {
    perror("Could not create outgoing socket.");
    return socketOut;
  }

  // Convert serverM host
  struct sockaddr_in servAddr;
  servAddr.sin_port = htons(port);
  status = inet_aton(localhost, &servAddr.sin_addr);
  if (!status) {
    perror("Could not convert backend host address.");
    return status;
  }

  // Send datagram
  status = sendto(socketOut, req, sizeof(req), 0, 
    (struct sockaddr *)&servAddr, sizeof(struct sockaddr_in));
  if (status < 0) {
    perror("Error sending datagram to backend");
    return status;
  }

  // Await response, filling out data based on type of request sent
  struct sockaddr incomingAddr;
  socklen_t incomingAddrSize = sizeof(incomingAddr);

  int numEntries = 0;
  serverResponse res;
  transaction t;

  while (1) {
    status = recvfrom(socketParents[2], &res, sizeof(res), 
      0, &incomingAddr, &incomingAddrSize);
    if (status < 0) {
      perror("Error receiving UDP message");
      return status;
    }
    
    // Process message. Continue reading messages until we arrive at a message
    // with "finalResponse" field set to true.
    if (res.finalResponse) break;
 
    if (res.responseType == SERVER_CHECK) {
      parseTransaction(res.transaction, &t);
      memcpy(log + numEntries, &t, sizeof(t));
    }
    numEntries++;
  }

  status = close(socketOut);
  if (status) {
    perror("Error closing socket.");
    return status;
  }

  return numEntries; 
}

/* Takes a passed transaction string "str", and parses int into "entry" */
void parseTransaction(char *str, transaction *entry) {
  char *token;
  token = strtok(str, " ");
  entry->transactionID = atoi(token);
  token = strtok(NULL, " ");
  strcpy(entry->sender, token);
  token = strtok(NULL, " ");
  strcpy(entry->receiver, token);
  token = strtok(NULL, " ");
  entry->amt = atoi(token);
}

/* Main driver */
int main() {
  int status = 0;

  status = initializeServerSockets();
  if (status) return status;

  printBootUp();

  status = acceptClientConnections();
  if (status) return status;

  return status;
}
