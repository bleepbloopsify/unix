#include <signal.h>
#include <stdio.h>
#include <setjmp.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


sigjmp_buf env;

void sighandler(int sig) {
  puts("Event handler");
  siglongjmp(env, 1);
}

int main () {
  pid_t p;
  sigset_t blah, old_set;
  int jumped = 0;

  sigsetjmp(env, 1);
  printf("jumped: %d\n", jumped);
  if (jumped++ > 4) exit(0);
  puts("Forking");

  sigemptyset(&blah);
  sigaddset(&blah, SIGINT);
  sigprocmask(SIG_BLOCK, &blah, NULL);

  p = fork();
  if (p == 0) {
    puts("child process");
    signal(SIGINT, sighandler);
    sigprocmask(SIG_UNBLOCK, &blah, NULL);
    puts("Proc finished!");
    for(;;) {}    
  }

  puts("killing process");

  sigsetjmp(env, 1);
  puts("hello there");
  kill(p, SIGINT);
  sigprocmask(SIG_UNBLOCK, &blah, NULL);

  puts("Ok killed");
  for(;;) {}
}