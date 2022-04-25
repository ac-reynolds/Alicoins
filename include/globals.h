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

// Between client and main server
#define CLIENT_CHECK 1
#define CLIENT_TRANSFER 2
#define CLIENT_TXLIST 3
#define CLIENT_STATS 4

// Between main server and backend
#define SERVER_CHECK 1
#define SERVER_TRANSFER 2

// For backend UDP connections
#define TALKER 0
#define LISTENER 1

#define MAX_NAME_LENGTH 512
#define MAX_TRANSACTION_LENGTH 1024
#define MAX_NUM_TRANSACTIONS 3000
#define INITIAL_ACCOUNT_VALUE 1000
#define MAX_USERS_IN_NETWORK 100

// for stat check
typedef struct StatEntry {
  int rank;
  char username[MAX_NAME_LENGTH];
  int numTX;
  int netBalance;
} statEntry;

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
  unsigned short senderPresent;
  unsigned short receiverPresent;
  unsigned short insufficientFunds;
  statEntry userStats[MAX_USERS_IN_NETWORK];
  unsigned short numUsers;
} clientResponse;

// from serverM to serverX
typedef struct ServerRequest {
  unsigned short requestType;
  unsigned short terminalOperation; // Should the backend server print output after servicing this request?
  char transaction[MAX_TRANSACTION_LENGTH]; // optional, should include newline suffix
} serverRequest;

// from serverX to serverM, many of these responses may be sent
typedef struct ServerResponse {
  unsigned short responseType;
  unsigned short confirmation; // after a transaction is successfully added
  unsigned short finalResponse; // to indicate that no more bits should be listened for
  char transaction[MAX_TRANSACTION_LENGTH];
} serverResponse;

