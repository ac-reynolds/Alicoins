#include "clientB.h"

/* Messages */
void printBootUp() {
  printf("The client %c is up and running.\n", CLIENT_ID);
}

void printUserCheck(char *usr) {
  printf("\"%s\" sent a balance enquiry request to the main server.\n", usr);
}

void printUserTransfer(char *sender, char *receiver, int amt) {
  printf("\"%s\" has requested to transfer %d coins to \"%s\".\n", sender, amt, receiver);
}

void printUserBalance(char *usr, int balance) {
  printf("The current balance of \"%s\" is: %d alicoins.\n", usr, balance);
}

void printTransferSuccess(char *sender, char *receiver, int amt, int balance) {
  printf("\"%s\" successfully transferred %d to \"%s\".\nThe current balance of \"%s\" is %d alicoins.\n", sender, amt, receiver, sender, balance);
}

void printFailureNotEnoughFunds(char *sender, char *receiver, int amt, int balance) {
  printf("\"%s\" was unable to transfer %d to \"%s\" because of insufficient balance.\nThe current balance of \"%s\" is: %d alicoins.\n", sender, amt, receiver, sender, balance);
}

void printFailureUserDNE(char *usr) {
  printf("Unable to proceed with the transaction as \"%s\" is not part of the network.\n", usr);
}

void printFailureUsersDNE(char *usr1, char *usr2) {
  printf("Unable to proceed with the transaction as \"%s\" and \"%s\" are not part of the network.\n", usr1, usr2);
}

void printUserTXLIST() {
  printf("Client%c sent a sorted list request to the main server.\n", CLIENT_ID);
}

void printUserStats(char *usr) {
  printf("\"%s\" sent a statistics enquiry request to the main server.\n", usr);
}

void printStatsHeader(char *usr) {
  printf("\"%s\" statistics are the following.:\n", usr);
  printf("  Rank --- Username --- #TXWithUser --- XferAmt\n"); 
}

void printStatsLine(statEntry *stats) {
  printf(" %-6d | %-10s | %-13d | %-8d\n", stats->rank, stats->username, stats->numTX, stats->netBalance);
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
  servAddr.sin_port = htons(ASSIGNED_PORT);
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

/* Perform request/response action. */
int requestServerResponse(clientRequest *req, clientResponse *res) {
  int status = 0;

  status = send(sockfd, req, sizeof(clientRequest), 0);
  if (status < 0) {
    perror("Could not send request to server");
    return status;
  }

  status = recv(sockfd, res, sizeof(clientResponse), 0);
  if (status < 0) return status;

  return 0;
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
int doUserCheck(char* usr) {
  int status = 0;

  // Set up message
  clientRequest req;
  req.requestType = CLIENT_CHECK;
  strncpy(req.sender, usr, MAX_NAME_LENGTH);

  clientResponse res;

  // Send message
  connectToServer();
  printUserCheck(usr);

  status = requestServerResponse(&req, &res);
  if (status) return status;

  // Print appropriate response based on message
  if (res.senderPresent) {
    printUserBalance(req.sender, res.result);
  } else {
    printFailureUserDNE(req.sender);
  }

  disconnectFromServer();
  return status;
}

/* Perform transfer operation */
int doUserTransfer(char* sender, char *receiver, int amt) {
  int status = 0;

  // Set up message
  clientRequest req;
  req.requestType = CLIENT_TRANSFER;
  strncpy(req.sender, sender, MAX_NAME_LENGTH);
  strncpy(req.receiver, receiver, MAX_NAME_LENGTH);
  req.amt = amt;

  clientResponse res;

  // Send message
  connectToServer();
  printUserTransfer(sender, receiver, amt);

  status = requestServerResponse(&req, &res);
  if (status) return status;

  // Print appropriate response based on message
  if (!res.senderPresent && !res.receiverPresent) {
    printFailureUsersDNE(req.sender, req.receiver);
  } else if (!res.senderPresent) {
    printFailureUserDNE(req.sender);
  } else if (!res.receiverPresent) {
    printFailureUserDNE(req.receiver);
  } else if (res.insufficientFunds) {
    printFailureNotEnoughFunds(req.sender, req.receiver, req.amt, res.result);
  } else { // success 
    printTransferSuccess(req.sender, req.receiver, req.amt, res.result);
  }

  disconnectFromServer();
  return status;
}

/* Send message for creating transaction list */
int doTXLIST() {
  int status = 0;

  // Set up message
  clientRequest req;
  req.requestType = CLIENT_TXLIST;

  clientResponse res;

  // Send message
  connectToServer();
  printUserTXLIST();

  status = requestServerResponse(&req, &res);
  if (status) return status;

  disconnectFromServer();
  return status;
}

/* Send message for stats check */
int doStats(char *usr) {
  int status = 0;

  // Set up message
  clientRequest req;
  req.requestType = CLIENT_STATS;
  strcpy(req.sender, usr);

  clientResponse res;

  // Send message
  connectToServer();
  printUserStats(usr);

  status = requestServerResponse(&req, &res);
  if (status) return status;

  disconnectFromServer();

  if (res.numUsers == 0) {
    printFailureUserDNE(usr);
    return 0;
  }

  printStatsHeader(usr);
  for (int i = 0; i < res.numUsers; i++) {
    printStatsLine(res.userStats + i);
  }

  return status;
}

/* Parses commands, does not error check. */
int main(int argc, char** argv) {
  if (argc == 2) {
    
    // differentiates between the two 1-argument commands
    if (strcmp(argv[1], "TXLIST")) {
      return doUserCheck(argv[1]);
    } else {
      return doTXLIST();
    }

  } else if (argc == 3) {
    return doStats(argv[1]);
  } else if (argc == 4) {
    return doUserTransfer(argv[1], argv[2], strtol(argv[3], NULL, 10));
  } 
  return 0;
}
