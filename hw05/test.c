#include <stdio.h>
#include <signal.h>

void interruptHandler(int signum) {
  printf("Received signal %d from process\n", signum);
}

int main() {
  struct sigaction action;
  action.sa_handler = interruptHandler;
  sigemptyset(&action.sa_mask);

  sigaction(SIGINT, &action, NULL);

  while(1) {};
}