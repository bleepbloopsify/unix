#include <pthread.h>
#include <stdio.h>

int global = 0;

void* incme(void* _) {

  for (size_t i = 0; i < 1000000000; ++i) {
    __asm__ volatile (
      "lock incl (%0)\n"
      :
      : "r"(&global)
    );
  }

  return NULL;
}

int main() {

  pthread_t tid1, tid2;

  pthread_create(&tid1, NULL, incme, NULL);
  pthread_create(&tid2, NULL, incme, NULL);

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);


  printf("global: %d\n", global);
}