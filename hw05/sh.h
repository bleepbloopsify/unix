#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

#define DEFAULT_PROMPT "$ "
#define PROMPT_DELIM " "
#define INITIAL_PROCESS_LIST_SIZE 10
#define INITIAL_ARGV_SIZE 10
#define PIPE_DELIM "|"
#define DEFAULT_CD_ARG "~"

typedef struct {
  char** argv;
  int _stdin, _stdout, _stderr;
  pid_t pid;
} Process;

Process** processes = NULL;
sigjmp_buf env;

int main();
char* getPrompt();
char* readLine(char*);
int prepareForRedirection(char*, int, int);
Process* makeProcess(char*);
Process** makeProcesses(char*);
void printArgs(Process*);
void runProcess(Process*);
void forkAndExec(Process*);
void interruptHandler(int);
void quitHandler(int);