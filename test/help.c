#include <stdio.h>
#include <signal.h>
#include <pthread.h>

int x = 0;

int main();
void signalHandler(int, siginfo_t*, void*);
void* pthread_function(void*);
void* pthread_signal(void*);

int main() {
  pthread_t my_id;
  struct sigaction action;
  pthread_t threads[0x11];
  sigset_t blockme, old;

  my_id = pthread_self();

  sigemptyset(&blockme);
  sigaddset(&blockme, SIGINT);

  printf("Main's pthread id is: %p\n", my_id);

  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, SIGINT);

  action.sa_sigaction = signalHandler;

  sigaction(SIGINT, &action, NULL);
  sigprocmask(SIG_BLOCK, &blockme, &old);

  for (size_t i = 0; i < 0x10; ++i) {
    pthread_create(&threads[i], NULL, pthread_function, NULL);
  }

  pthread_create(&threads[0x10], NULL, pthread_signal, NULL);

  sigprocmask(SIG_UNBLOCK, &blockme, &old);

  pthread_exit(0);
}

void signalHandler(int signum, siginfo_t* act, void* _) {
  pthread_t my_id;

  my_id = pthread_self();
  printf("Hello friends from thread %p!\n", my_id);

  ++x;
  printf("This is the %d time this has been called\n", x);
  pthread_exit(0);
}

void* pthread_function(void* _) {
  pthread_t my_id;

  my_id = pthread_self();

  printf("My pthread id is: %p\n", my_id);

  for(;;) {
  }

  return NULL;
}

void* pthread_signal(void* _) {
  for(;;) {
    raise(SIGINT);
  }

  return NULL;
}