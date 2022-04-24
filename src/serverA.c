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
  for (int i = 0; i < 2; i++) {
    sockets[i] = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockets[i] < 0) {
      perror("Could not create UDP socket.\n");
      return sockets[i];
    }
  }

  // Set socket addr to be reused (no zombies!)
  int optval = 1;
  status = setsockopt(sockets[LISTENER], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (status) {
    perror("Could not set UDP socket option.\n");
  }

  // Convert host address at main server
  mainServAddr.sin_port = htons(SX2SMPort);
  status = inet_aton(localhost, &mainServAddr.sin_addr);
  if (!status) {
    perror("Could not convert main server host address");
    return status;
  }  

  // Convert host address at this server
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

/* sends response to main server */
int sendResponse(serverResponse *res) {
  int status = 0;

  // Send datagram
  status = sendto(sockets[TALKER], res, sizeof(serverResponse), 0,
    (struct sockaddr *)&mainServAddr, sizeof(struct sockaddr_in));
  if (status < 0) {
    perror("Error sending datagram");
    return status;
  }

  return status;
}

/* */
int handleRequest(serverRequest* req) {
  int status = 0;
  char line[MAX_TRANSACTION_LENGTH] = "5 Chinmay Oliver 129\n";
  FILE *f;
  char word2[60] = "1 Racheal John 45";
  serverResponse res;

  switch (req->requestType) {
    case SERVER_CHECK:
      if (req->terminalOperation) printRequest();

      // Set response fields
      res.responseType = SERVER_CHECK;
      res.finalResponse = FALSE;

      // Get file contents, one line at a time
      f = fopen(BLOCK_FILE_NAME, "r");
      if (f < 0) return -1;

      // Read through entire text file, sending each non-empty line to the main server
      while (fgets(res.transaction, sizeof(line), f) != NULL) {
        if (res.transaction[0] == '\0' || res.transaction[0] == '\n') continue;

        // Send full transaction line
        status = sendResponse(&res);
        if (status < 0) return status;
      }
      status = fclose(f);
      if (status) return status;

      // Send final message to tell server to stop waiting for more transactions
      res.finalResponse = TRUE;
      status = sendResponse(&res);
      if (status < 0) return status;
      
      if (req->terminalOperation) printResponse();
      break;

    case SERVER_TRANSFER:
      printRequest();

      res.responseType = SERVER_TRANSFER;
      res.confirmation = TRUE;
      res.finalResponse = TRUE;

      // Get file contents, one line at a time
      f = fopen(BLOCK_FILE_NAME, "a");
      if (f < 0) return -1;

      fprintf(f, "%s\n", req->transaction);

      status = fclose(f);
      if (status) return status;

      status = sendResponse(&res);
      printResponse();
      break;
  }  
  
  return 0;
}

int receiveRequests() {
  int status = 0;
  socklen_t incomingAddrSize;
  struct sockaddr incomingAddr;
  incomingAddrSize = sizeof(incomingAddr);
  serverRequest req;

  // Loop forever, accepting incoming request from main server
  while(1) {
    status = recvfrom(sockets[LISTENER], &req, sizeof(serverRequest), 0, &incomingAddr, &incomingAddrSize);
    if (status < 0) return status;
    status = handleRequest(&req);
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

  status = receiveRequests();
  if (status) return status;

  return status;
}
