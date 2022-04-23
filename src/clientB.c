#include "clientB.h"

/* Messages */
void printBootUp() {
  printf("The client %c is up and running.\n", clientID);
}

void printUserCheck(char* usr) {
  printf("%s sent a balance enquiry request to the main server.\n", usr);
}

void printUserTransfer(char* sender, char* receiver, int amt) {
  printf("%s has requested to transfer %d coins to %s.\n", sender, amt, receiver);
}

/* Initializes a TCP socket and connect to serverM */
int connectToServer() {
  int status = 0;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Could not create socket.\n");
    return sockfd;
  }

  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(assignedPort);
  status = inet_aton(localhost, &servAddr.sin_addr);
  if (!status) {
    perror("Could not convert host address.\n");
    return status;
  }

  status = connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
  if (status) {
    perror("Error when connecting to server.\n");
    return status;
  }
  
  return status;
}

/* Close socket */
int disconnectFromServer() {
  int status = close(sockfd);
  if (status) {
    perror("Error closing socket.\n");
    return status;
  }
}

/* Perform user check */
int userCheck(char* usr) {
  int status = 0;
  connectToServer();

  // Set up message
  checkUserRequest req;
  strncpy(req.name, usr, MAX_NAME_LENGTH);

  printUserCheck(usr);
  status = send(sockfd, (void *)&req, sizeof(req), 0);

  disconnectFromServer();
  return 0;
}

int userTransfer(char* sender, char* receiver, int amt) {
  connectToServer();
  printUserTransfer(sender, receiver, amt);
  disconnectFromServer();
  return 0;
}

/* Parses commands, does not error check. */
int main(int argc, char** argv) {
  if (argc == 2) {
    
    // differentiates between the two 1-argument commands
    if (strcmp(argv[1], "TXLIST")) {
      return userCheck(argv[1]);
    }

  } else if (argc == 4) {
    return userTransfer(argv[1], argv[2], strtol(argv[3], NULL, 10));
  } 
  return 0;
}
