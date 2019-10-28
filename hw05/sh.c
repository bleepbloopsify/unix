#include "sh.h"


int main() {
  struct sigaction int_action, quit_action;
  sigset_t new_set, old_set;
  char* prompt, *input, *cmd;
  Process** curr;
  int wstatus;
  pid_t pid, err;

  int_action.sa_handler = interruptHandler;
  sigemptyset(&int_action.sa_mask);
  int_action.sa_flags = SIGQUIT;

  sigaction(SIGINT, &int_action, NULL);

  quit_action.sa_handler = quitHandler;
  sigemptyset(&quit_action.sa_mask);
  quit_action.sa_flags = SIGINT;

  sigaction(SIGQUIT, &quit_action, NULL);

  sigemptyset(&new_set);
  sigaddset(&new_set, SIGQUIT);
  sigaddset(&new_set, SIGINT);

  while(1) {
    sigsetjmp(env, 1); // so we can jump here in case of an interrupt

    prompt = getPrompt();
    input = readLine(prompt);

    if (input[0] == 0) { // nothing on line
      continue; // nothing to do
    }

    curr = processes = makeProcesses(input);

    if (processes == NULL) {
      continue;
    }

    do {
      runProcess(*curr);
    } while(*(++curr) != NULL);

    curr = processes;

    do {
      pid = (*curr)->pid;

      // printf("waiting for pid %d with executable %s\n", pid, (*curr)->argv[0]);

      while(1) {
        err = waitpid(pid, &wstatus, 0);
        if (err == -1) {
          // fprintf(stderr, "Could not wait for child with pid %d\n", pid);
          if (errno == ECHILD) break; // cannot wait for nonexistent child
        } else break;
      }

      // printf("Process %d exited with status code %d\n", pid, wstatus);
    } while(*(++curr) != NULL);

    // we would LIKE to free our processes without being interrupted

    sigprocmask(SIG_BLOCK, &new_set, &old_set);
    curr = processes;
    do {
      free(*curr);
    } while(*(++curr) != NULL);
    free(processes);
    processes = NULL;
    free(input); // the processes are using this buffer for argv

    sigprocmask(SIG_UNBLOCK, &new_set, NULL);
  }

  return 0;
}

char* getPrompt() {
  char* ps1;

  ps1 = getenv("PS1");
  if (ps1 == NULL) {
    return DEFAULT_PROMPT;
  }

  return ps1;
}

char* readLine(char* prompt) {
  char* line = NULL;
  size_t len = 0, read;

  printf("%s", prompt);

  read = getline(&line, &len, stdin);
  if (read == -1) {
    // fprintf(stderr, "Error reading from command line\n");
    exit(1); // we have to exit if we can't read input.
  }

  line[read - 1] = 0;// we don't need the newline

  return line;
}

Process* makeProcess(char* prompt) {
  Process* process = NULL;
  char* token = strtok(prompt, PROMPT_DELIM);
  char* fname = NULL; // for redirection
  int err, argc = 0, argv_capacity;
  char** victim;

  if (token == NULL) {
    return process;
  }

  process = malloc(sizeof(Process));
  if (process == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  process->argv = malloc(sizeof(char*) * INITIAL_ARGV_SIZE);
  if (process->argv == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  process->_stdin = -1;
  process->_stdout = -1;
  process->_stderr = -1;
  process->pid = -1;
  argv_capacity = INITIAL_ARGV_SIZE;

  do {
    if (strcmp(token, ">") == 0) { // redirect stdout
      fname = strtok(NULL, PROMPT_DELIM);
      
      process->_stdout = prepareForRedirection(fname, O_RDWR | O_CREAT, 0666);
    } else if (strcmp(token, "2>") == 0) {
      fname = strtok(NULL, PROMPT_DELIM);
      
      process->_stderr = prepareForRedirection(fname, O_RDWR | O_CREAT, 0666);
    } else if (strcmp(token, ">>") == 0) {
      fname = strtok(NULL, PROMPT_DELIM);
      
      process->_stdout = prepareForRedirection(fname, O_RDWR | O_CREAT, 0666);
      lseek(process->_stdout, 0, SEEK_END);
      // ignore error here it doesn't matter
    } else if (strcmp(token, "<") == 0) {
      fname = strtok(NULL, PROMPT_DELIM);
      
      process->_stdin = prepareForRedirection(fname, O_RDONLY, 0);
    } else {
      process->argv[argc++] = token;
      
      if(argc >= argv_capacity) { // expandable argv!
        victim = process->argv;
        process->argv = malloc(sizeof(char*) * argv_capacity * 2);
        if (process->argv == NULL) {
          fprintf(stderr, "Out of memory\n");
          exit(1);
        }
        argv_capacity *= 2;
        for (size_t i = 0; i < argc; ++i) {
          process->argv[i] = victim[i];
        }
        free(victim);
      }
    }
  } while((token = strtok(NULL, PROMPT_DELIM)));

  process->argv[argc] = NULL;

  return process;
}

int prepareForRedirection(char* fname, int mode, int perms) {
  int new_fd;

  if (fname == NULL) {
    fprintf(stderr, "Bad syntax, no file specified\n");
    exit(1);
  }

  new_fd = open(fname, mode, perms);
  if (new_fd == -1) {
    fprintf(stderr, "Error opening file %s", fname);
  }

  return new_fd;
}

void printArgs(Process* process) {
  char** arg = process->argv;

  printf("Process args are: %s", *arg);

  while(*(++arg)) {
    printf(", %s", *arg);
  }

  printf("\n");
}

Process** makeProcesses(char* prompt) {
  Process** processes = NULL, **victim;
  Process* process, *previous;
  char* token, *saveptr;
  size_t procc, proc_capacity; // like argc but processes
  int new_pipe[2], err;

  token = strtok_r(prompt, PIPE_DELIM, &saveptr);

  if (token == NULL) {
    return processes;
  }

  processes = malloc(sizeof(Process*) * INITIAL_PROCESS_LIST_SIZE);
  if (processes == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  proc_capacity = INITIAL_PROCESS_LIST_SIZE;
  procc = 0;

  do {
    process = makeProcess(token);
    if (process == NULL) { // token is empty?
      break;
    }

    processes[procc++] = process;

    if (procc > 1) {
      previous = processes[procc - 2];
      // not currently redirecting  
      if (previous->_stdout == -1 && process->_stdin == -1) { 
        err = pipe(new_pipe);
        if (err == -1) {
          fprintf(stderr, "Error creating pipe\n");
        } else {
          previous->_stdout = new_pipe[1];
          process->_stdin = new_pipe[0];
        }
      }
    }

    if (procc > proc_capacity) {
      victim = processes;
      processes = malloc(sizeof(Process*) * 2 * proc_capacity);
      if (processes == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
      }
      proc_capacity *= 2;
      for (size_t i = 0; i < procc; ++i) {
        processes[i] = victim[i];
      }
      free(victim);
    }
  } while((token = strtok_r(NULL, PIPE_DELIM, &saveptr)));

  processes[procc] = NULL;

  return processes;
} 

void runProcess(Process* process) {
  char* cmd, *arg;
  int err;

  cmd = process->argv[0];
  if (cmd == NULL) return;
  if (strcmp(cmd, "cd") == 0) {
    arg = process->argv[1];
    if (arg == NULL || strcmp(arg, "~") == 0) arg = getenv("HOME");
    if (arg == NULL) {
      fprintf(stderr, "$HOME variable not set");
      return;
    }

    err = chdir(arg);
    if (err == -1) {
      fprintf(stderr, "Could not cd to %s", arg);
    }

    return;
  } else if (strcmp(cmd, "exit") == 0) {
    exit(0);
  } else {
    forkAndExec(process);
  }
}

void forkAndExec(Process* process) {
  pid_t child;
  int err;

  process->pid = child = fork();
  if (child == -1) {
    fprintf(stderr, "Error creating child process\n");
    return;
  }

  if (child == 0) {
    if (process->_stdin != -1) {
      err = dup2(process->_stdin, STDIN_FILENO);
      if (err == -1) fprintf(stderr, "Could not redirect stdin\n");
    }
    if (process->_stdout != -1) {
      err = dup2(process->_stdout, STDOUT_FILENO);
      if (err == -1) fprintf(stderr, "Could not redirect stdout\n");
    }
    if (process->_stderr != -1) {
      err = dup2(process->_stderr, STDERR_FILENO);
      if (err == -1) fprintf(stderr, "Could not redirect stderr\n");
    }

    err = execvp(process->argv[0], process->argv);
    if (err == -1) {
      fprintf(stderr, "Could not execute program %s", process->argv[0]);
      exit(1);
    }
  }
}

void interruptHandler(int signum) {
  Process** curr = processes;

  if (curr == NULL) siglongjmp(env, 0); // shells don't exit on SIGINT
  else do {
    kill((*curr)->pid, signum);
  } while(*(++curr) != NULL);
}

void quitHandler(int signum) {
  Process** curr = processes;

  if (curr == NULL) exit(0);
  else do {
    kill((*curr)->pid, signum);
  } while (*(++curr) != NULL);
}