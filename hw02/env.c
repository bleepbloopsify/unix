/**
 * Leon Chou
 * lc3488
 * N17903238 
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <errno.h>


extern char** environ;


void printEnv(char**);
int countEnvvars(int argc, char** argv);
char** makeArgs(int argc, char* argv[]);
char** mergeEnv(int envc, char** envvars);
int main(int argc, char* argv[]);


// utility print function using NULL ptr terminated arrays.
void printEnv(char** env) {
  char** curr;

  curr = env;
  while(*curr) {
    printf("%s\n", *curr++);
  }
}


// count how many environment variables exist in argv, using existence of = sign.
int countEnvvars(int argc, char** argv) {
  char* arg;
  size_t count_envvars = 0;

  for (size_t idx = 0; idx < argc; ++idx) {
    arg = argv[idx];

    if (strchr(arg, '=') != NULL) {
      ++count_envvars;
      continue;
    }

    break;
  }

  return count_envvars; // this represents how many of the arguments are actually envvars.
}


// create an argument list for execvpe
char** makeArgs(int argc, char* argv[]) {
  char** args = (char**)malloc(sizeof(char*) * (argc + 1)); // one more for the NULL ptr
  if (args == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(errno);
  }

  for (size_t idx = 0; idx < argc; ++idx) {
    args[idx] = argv[idx];
  }

  args[argc] = NULL;

  return args;
}


// creates a new valid array on the heap using current environ and new specified envvars.
char** mergeEnv(int envc, char** envvars) {
  char** curr;
  char** newenv;
  size_t idx = 0, oldenvc = 0;

  curr = environ;
  while(*curr++) ++oldenvc; // we increment envc to account for the fact that we already have environment vars

  newenv = (char**)malloc(sizeof(char*) * (oldenvc + envc + 1)); // one more for the NULL ptr
  if (newenv == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(errno);
  }

  curr = environ;
  while (*curr) {
    newenv[idx++] = *curr++;
  }

  for (size_t envidx = 0; envidx < envc; ++envidx) {
    newenv[idx++] = envvars[envidx];
  }

  newenv[oldenvc + envc + 1] = NULL;

  return newenv;
}


// usage: env [-i] [envvars] [command [arguments]]
int main(int argc, char* argv[]) {
  int number_of_envvars, err;
  char** args;
  char** newenv;

  if (argc < 2) { // just print all envvars
    printEnv(environ);

    return 0; // we're out
  }

  if (strcmp(argv[1], "-i") == 0) { // the initial flag is set
    number_of_envvars = countEnvvars(argc - 2, &argv[2]);

    if (number_of_envvars == argc - 2) { // nothing but envvars and we also don't have any command to run
      for (size_t idx = 0; idx < number_of_envvars; ++idx) {
        printf("%s\n", argv[2 + idx]);
      }

      return 0; // this is all we gotta do
    }
  
    args = makeArgs(argc - number_of_envvars - 2, &argv[2 + number_of_envvars]);
    // we clobber argv to make it easy to construct the new environment
    argv[2 + number_of_envvars] = NULL;

    // we don't have to free here because execvpe will clobber all of memory anyways
    err = execvpe(args[0], args, &argv[2]);
    printf("%d\n", err);

    return 0;
  }

  number_of_envvars = countEnvvars(argc - 1, &argv[1]); // offset by one

  if (number_of_envvars == argc - 1) {
    printEnv(environ);

    for (size_t idx = 0; idx < number_of_envvars; ++idx) {
      printf("%s\n", argv[1 + idx]);
    }

    return 0; // this is all we gotta do
  }

  args = makeArgs(argc - number_of_envvars - 1, &argv[1 + number_of_envvars]);
  newenv = mergeEnv(number_of_envvars, &argv[1]);

  // we don't have to free here because execvpe will replace the entire heap
  err = execvpe(args[0], args, newenv);

  return 0;
}