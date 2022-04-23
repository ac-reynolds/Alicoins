#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0

#define localhost "127.0.0.1"

#define SM2SAPort 21840
#define SM2SBPort 22840
#define SM2SCPort 23840
#define SX2SMPort 24840
#define CA2SMPort 25840
#define CB2SMPort 26840

#define INDEX_TO_ID_OFFSET 65 // ascii code for "A"

#define MAX_NAME_LENGTH 512
#define MAX_TRANSACTION_LENGTH 1024

// Between client and main server
#define CLIENT_CHECK 10
#define CLIENT_TRANSFER 11

// Between main server and backend
#define SERVER_CHECK 20
#define SERVER_TRANSFER 21

// For backend UDP connections
#define TALKER 0
#define LISTENER 1

#define MAX_NUM_TRANSACTIONS 3000
#define MAX_FILE_LENGTH 65507

// from clientX to serverM
typedef struct ClientRequest {
  unsigned short requestType;
  char sender[MAX_NAME_LENGTH];
  char receiver[MAX_NAME_LENGTH]; //optional
  int amt; //optional
} clientRequest;

// from serverM to clientX
typedef struct ClientResponse {
  unsigned short responseType;
  int result;
} clientResponse;

// from serverM to serverX
typedef struct ServerRequest {
  unsigned short requestType;
  unsigned short terminalOperation; // Should the backend server print output after servicing this request?
  char transaction[MAX_TRANSACTION_LENGTH]; // optional, should include newline suffix
} serverRequest;

// server responses are just char[] and int
