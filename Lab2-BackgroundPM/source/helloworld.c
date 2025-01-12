#include <stdio.h>
#include <string.h>
#include "./ProcessManager/ProcessManager.h"

int main() {
  char command[256];

  printf("Enter command to execute: ");
  if (fgets(command, sizeof(command), stdin) == NULL) {
    fprintf(stderr, "Command input error.\n");
    return 1;
  }

  int exit_code = Run(command);
  
  if (exit_code == -1) {
    fprintf(stderr, "Child process launch error.\n");
  } else {
    printf("Child process ended with the code: %d\n", exit_code);
  }
    
  printf("The main process is completed.\n");

  return 0;
}