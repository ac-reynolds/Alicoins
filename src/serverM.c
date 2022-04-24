#include "serverM.h"

/* Messages */
void printBootUp() { 
  printf("The main server is up and running.\n");
}

void printUserCheck(char* usr, int port) {
  printf("The main server received input=\"%s\" from the client using TCP over port %d.\n", usr, port);
}

void printUserTransfer(char* sender, char* receiver, int amt, int port) {
  printf("The main server received from \"%s\" to transfer %d coins to \"%s\" using TCP over port %d.\n", sender, amt, receiver, port);
}

void printBackendRequest(char x) {
  printf("The main server sent a request to server %c.\n", x);
}

void printBackendCheckResponse(char x, int port) {
  printf("The main server received transactions from Server %c using UDP over port %d.\n", x, port);
}

void printBackendTransferResponse(char x, int port) {
  printf("The main server received the feedback from server %c using UDP over port %d.\n", x, port);
}

void printClientCheckResponse(char x) {
  printf("The main server sent the current balance to client %c.\n", x);
}

void printClientTransferResponse(char x) {
  printf("The main server sent the result of the transaction to client %c.\n", x);
}

/* For debugging/utility */
void printTransaction(transaction *t) {
  printf("TX%d: \"%s\" -> \"%s\" (amt = %d)\n", t->transactionID, t->sender, t->receiver, t->amt);
}

/* Interprets the client message. Sets res to the response to be sent back to the client. */
int handleClientMessage(char serverID, int sockfd, unsigned short port, clientResponse *res) {
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
      if (status < 0) return status;
      printClientCheckResponse(serverID);
      break;
    case CLIENT_TRANSFER:
      printUserTransfer(req.sender, req.receiver, req.amt, port);
      status = doUserTransfer(req.sender, req.receiver, req.amt, res);
      if (status) return status;
      status = send(sockfd, res, sizeof(clientResponse), 0);
      if (status < 0) return status;
      printClientTransferResponse(serverID);
      break;
  }

  return 0;
}

/* Retrieves the full log from the backend */
int getLog(transaction *log, int terminalOperation) {
  int status = 0;

  // Set up request
  serverRequest req;
  req.requestType = SERVER_CHECK;
  req.terminalOperation = terminalOperation;

  // Fill in entries into table based on backend data
  int tableSize = 0;
  for (int i = 0; i < 3; i++) {
    status = requestBackendResponse(i + INDEX_TO_ID_OFFSET, PORTS_TO_SERVERS[i], &req, log + tableSize);
    if (status < 0) return status;
    tableSize += status;
  }

  return tableSize;
}

/* Push transaction */
int pushTransaction(transaction *t) {
  int status = 0;
  int choice;
  transaction *unused;

  // Set up request
  serverRequest req;
  req.requestType = SERVER_TRANSFER;
  req.terminalOperation = TRUE;
  stringifyTransaction(t, req.transaction);

  // Pick a random server to send to
  choice = rand() % 3;

  // Fire off request, await response
  status = requestBackendResponse(choice + INDEX_TO_ID_OFFSET,
    PORTS_TO_SERVERS[choice], &req, unused);
  if (status < 0) return status;
  
  return 0;
}

/* Perform the check functionality */
int doUserCheck(char* name, clientResponse *res) {
  int status = 0;
  int numEntries;

  // initialize transaction table
  transaction log[MAX_NUM_TRANSACTIONS];
  numEntries = getLog(log, TRUE);
  if (numEntries < 0) return status;

  // Calculate funds associated with the passed user  
  int userFunds = INITIAL_ACCOUNT_VALUE;
  int userFound = 0;
  for (int i = 0; i < numEntries; i++) {
    //printTransaction(log + i);
    if (!strcmp(name, (log + i)->sender)) {
      userFunds -= (log + i)->amt;
      userFound = 1;
    } 
    if (!strcmp(name, (log + i)->receiver)) {
      userFunds += (log + i)->amt;
      userFound = 1;
    }
  }

  // Fill in response fields
  res->responseType = CLIENT_CHECK;
  res->result = userFunds;
  res->senderPresent = userFound;

  return 0;
}

/* Perform the transfer operation  */
int doUserTransfer(char *sender, char *receiver, int amt, clientResponse *res) {
  int status = 0;
  int numEntries;

  // initialize transaction table
  transaction log[MAX_NUM_TRANSACTIONS];
  numEntries = getLog(log, FALSE);
  if (numEntries < 0) return status;

  // Check to see that at sender and receiver both appear at least once
  int senderPresent = 0;
  int receiverPresent = 0;
  int senderFunds = INITIAL_ACCOUNT_VALUE;

  for (int i = 0; i < numEntries; i++) {

    // Verify that sender exists and tally up funds
    if (!strcmp(sender, (log + i)->sender)) {
      senderPresent = 1;
      senderFunds -= (log + i)->amt;
    }
    if (!strcmp(sender, (log + i)->receiver)) {
      senderPresent = 1;
      senderFunds += (log + i)->amt;
    }

    // Just verify that the receiver exists
    if (!strcmp(receiver, (log + i)->sender) || !strcmp(receiver, (log + i)->receiver)) {
      receiverPresent = 1;
    }
    //printTransaction(log + i);
  }

  // Fill in response fields
  res->responseType = CLIENT_TRANSFER;
  res->senderPresent = senderPresent;
  res->receiverPresent = receiverPresent;
  res->insufficientFunds = senderFunds < amt;
  res->result = res->insufficientFunds ? senderFunds : senderFunds - amt;  

  // send transaction if it passes error cases
  transaction t; 
  if (senderPresent && receiverPresent && !res->insufficientFunds) {

    // Transaction OK
    t.transactionID = numEntries + 1;
    strcpy(t.sender, sender);
    strcpy(t.receiver, receiver);
    t.amt = amt;
    status = pushTransaction(&t);
    if (status) return status;    

  }

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
      status = handleClientMessage(i + INDEX_TO_ID_OFFSET, incomingSockets[i], LISTENER_PORTS[i], &res);
      if (status) return status;
      close(incomingSockets[i]);
    }
  }
}

/* Fowards the request to the specified server. Records the response in the res field. */
int requestBackendResponse(char serverID, int port, serverRequest *req, transaction *log) {
  int status = 0;

  // Create socket to send UDP datagram
  int socketOut = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketOut < 0) {
    perror("Could not create outgoing socket.");
    return socketOut;
  }

  // Convert serverM host
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(port);
  status = inet_aton(localhost, &servAddr.sin_addr);
  if (!status) {
    perror("Could not convert backend host address.");
    return status;
  }

  // Send datagram
  status = sendto(socketOut, req, sizeof(serverRequest), 0, 
    (struct sockaddr *)&servAddr, sizeof(struct sockaddr_in));
  if (status < 0) {
    perror("Error sending datagram to backend");
    return status;
  }
  if (req->terminalOperation) printBackendRequest(serverID);

  // Await response, filling out data based on type of request sent
  struct sockaddr incomingAddr;
  socklen_t incomingAddrSize = sizeof(incomingAddr);

  int numEntries = 0;
  serverResponse res;
  transaction t;

    // For anything but a transfer push, we need to fill up the log
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

  if (!req->terminalOperation) {
  } else if(res.responseType == SERVER_CHECK) {
    printBackendCheckResponse(serverID, port);
  } else if (res.responseType == SERVER_TRANSFER) {
    printBackendTransferResponse(serverID, port);
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

/* Takes a passed transaction and converts it into a string. */
void stringifyTransaction(transaction *t, char *str) {
  //printTransaction(t);
  sprintf(str, "%d %s %s %d", t->transactionID, t->sender, t->receiver, t->amt);
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
