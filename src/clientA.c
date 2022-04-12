#include <globals.h>

/* Messages */
void printBootUp() {
  printf("The client A is up and running.\n");
}

void printUserCheck(char* usr) {
  printf("%s sent a balance enquiry request to the main server.\n", usr);
}

void printUserTransfer(char* sender, char* receiver, int amt) {
  printf("%s has requested to transfer %d coins to %s.\n", sender, amt, receiver);
}

int userCheck(char* usr) {
  printUserCheck(usr);
  return 0;
}

int userTransfer(char* sender, char* receiver, int amt) {
  printUserTransfer(sender, receiver, amt);
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
