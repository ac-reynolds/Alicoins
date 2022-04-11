#include <globals.h>

void printBootUp() {
  printf("The client A is up and running.\n");
}

void printUserCheck(char* usr) {
  printf("%s sent a balance enquiry request to the main server.\n", usr);
}

void printUserTransfer(char* sender, char* receiver, int amt) {
  printf("%s has requested to transfer %d coins to %s.\n", sender, amt, receiver);
}

int main() {
  printBootUp();
  return 0;
}
