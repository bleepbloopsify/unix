#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define DEFAULT_PS1 "$ "
#define LINE_TOKEN_SEP " "
#define LINE_MAX 4096 // my limits.h doesn't have LINE_MAX?
#define INITIAL_ARGC_SIZE 8
#define REDIRECT_STDOUT ">"
#define APPEND_STDOUT ">>"
#define REDIRECT_STDIN "<"
#define BUILTIN_CHDIR "cd"
#define BUILTIN_EXIT "exit"

// The redirection is left as fds because we will open files for use 
// during command parsing
typedef struct {
  char** args;
  int in;
  int out;
  int err;
} ChildProcess;

void print_prompt();
void take_n_input(char*, size_t);
ChildProcess create_process(char*);

// Fork and EXEC
int main() {
  int err, wstatus;
  ChildProcess process;
  char input_buf[LINE_MAX + 1]; // we allocate an extra byte for the null byte
  pid_t child;

  while(1) {
    print_prompt(); // this will print PS1 or use default if needed

    // this does not do buffered I/O because strtok requires an entire string
    take_n_input(input_buf, LINE_MAX); 

    // generates information about the process using strtok
    process = create_process(input_buf); 

    if (process.args == NULL) { // when the user presses enter
      continue;
    }

    if (strcmp(process.args[0], BUILTIN_CHDIR) == 0) {
      if (process.args[1] == NULL) { // cd to home dir!
        err = chdir("~");
      } else {
        err = chdir(process.args[1]);
      }

      if (err == -1) {
        fprintf(stderr, "[WARNING] Could not change into the "
          "specified directory! errno: %d\n", errno);
      }

      continue;
    }

    if (strcmp(process.args[0], BUILTIN_EXIT) == 0) {
      puts("OK bye!!");
      exit(0);
    }

    // we have something to execute, lets execute it a
    // nd wait for it to be finished.
    child = fork();
    if (child == -1) {
      fprintf(stderr, "[ERROR] Could not create child process\n");
      exit(errno);
    }
    
    if (child == 0) {

      if (process.in != -1) {
        err = dup2(process.in, STDIN_FILENO);
        if (err == -1) {
          fprintf(stderr, "[ERROR] Could not redirect stdin\n");
        }
      }

      if (process.out != -1) {
        err = dup2(process.out, STDOUT_FILENO);
        if (err == -1) {
          fprintf(stderr, "[ERROR] Could not redirect stdout\n");
        }
      }

      if (process.err != -1) {
        err = dup2(process.err, STDERR_FILENO);
        if (err == -1) {
          fprintf(stderr, "[ERROR] Could not redirect stderr\n");
        }
      }

      err = execvp(process.args[0], process.args);
      if (err == -1) {
        fprintf(stderr, "[ERROR] Could not exec program\n");
        exit(errno); // we exit here to let the parent know its a bad
      }

      break;
    }

    child = wait(&wstatus);
    if (child == -1) {
      fprintf(stderr, "There was an error waiting for the exec'd process\n");
      continue;
    }
    printf("Process %d exited with status code %d\n", child, wstatus);
  }

  return 0;
}

void print_prompt() {
  // We check PS1 each loop in case it changes
  char* ps1;
  int err;

  ps1 = getenv("PS1");

  if (ps1 == NULL) ps1 = DEFAULT_PS1;

  printf("%s", ps1);
  err = fflush(0);

  if (err < 0) {
    // not printing prompt is totally fine, not being 
    // able to flush stdout should be fine as well.
    // This is entirely recoverable.
    fprintf(stderr, "[WARNING] Error flushing stdout\n");
  }
}

void take_n_input(char* input_buf, size_t n) {
  int bytes_read;
  char* nl_loc;

  bytes_read = read(0, input_buf, LINE_MAX);

  nl_loc = strrchr(input_buf, '\n'); // we expect newlines to be at the end
  if (nl_loc == NULL) {
    input_buf[bytes_read] = 0;
  } else {
    *nl_loc = 0;
  } // basically remove the last newline byte
}

ChildProcess create_process(char* input_buf) {
  ChildProcess process;
  char** victim;
  char* fname;
  size_t argc = 0, command_capacity;
  char* token;
  off_t err;

  process.args = NULL;
  process.in = -1;
  process.out = -1;
  process.err = -1;

  token = strtok(input_buf, LINE_TOKEN_SEP);

  if (token == NULL) {
    return process;  
  }

  process.args = malloc(sizeof(char*) * INITIAL_ARGC_SIZE);
  command_capacity = INITIAL_ARGC_SIZE; 

  do { // as long as we keep getting tokens
    // we have to bundle up the previous tokens now
    if (strcmp(token, REDIRECT_STDOUT) == 0) { 
      fname = strtok(NULL, LINE_TOKEN_SEP);

      if (fname == NULL) { // there is NO filename
        fprintf(stderr, "[ERROR] Parse error of line, no file specified for "
        "redirection\n");
        exit(1);
      }

      err = remove(fname);
      if (err == -1) {
        if (errno != ENOENT) {
          fprintf(stderr, "[ERROR] Could not delete file to redirect to\n");
        }
      }

      if (argc > 0) { // we can grab the previous token.
        if (strcmp(process.args[argc - 1], "2") == 0) {
          process.err = open(fname, O_RDWR | O_CREAT, 0666);;

          if (process.err == -1) {
            fprintf(stderr, "[ERROR] Could not open %s\n", fname);
            exit(1);
          }
        } else {
          process.out = open(fname, O_RDWR | O_CREAT, 0666);

          if (process.out == -1) {
            fprintf(stderr, "[ERROR] Could not open %s\n", fname);
            exit(1);
          }
        }
      } else {
        process.out = open(fname, O_RDWR | O_CREAT, 0666);

        if (process.out == -1) {
          fprintf(stderr, "[ERROR] Could not open %s\n", fname);
          exit(1);
        }
      }
    } else if (strcmp(token, APPEND_STDOUT) == 0) {

      fname = strtok(NULL, LINE_TOKEN_SEP);

      if (fname == NULL) { // there is NO filename
        fprintf(stderr, "[ERROR] Parse error of line, no file specified for "
        "redirection\n");
        exit(1);
      }

      if (argc > 0) { // we can grab the previous token.
        if (strcmp(process.args[argc - 1], "2") == 0) {
          process.err = open(fname, O_RDWR, 0666);;

          if (process.err == -1) {
            fprintf(stderr, "[ERROR] Could not open %s\n", fname);
            exit(1);
          }

          err = lseek(process.err, 0, SEEK_END);
          if (err == -1) {
            fprintf(stderr, "[ERROR] Could not append to %s\n", fname);
            exit(1);
          }

          process.args[argc - 1] = NULL;
          argc -= 1;
        } else {
          process.out = open(fname, O_RDWR, 0666);

          if (process.out == -1) {
            fprintf(stderr, "[ERROR] Could not open %s\n", fname);
            exit(1);
          }

          err = lseek(process.out, 0, SEEK_END);
          if (err == -1) {
            fprintf(stderr, "[ERROR] Could not append to %s\n", fname);
            exit(1);
          }
        }
      } else {
        process.out = open(fname, O_RDWR | O_CREAT, 0666);

        if (process.out == -1) {
          fprintf(stderr, "[ERROR] Could not open %s\n", fname);
          exit(1);
        }

        err = lseek(process.out, 0, SEEK_END);
          if (err == -1) {
            fprintf(stderr, "[ERROR] Could not append to %s\n", fname);
            exit(1);
          }
      }

    } else if (strcmp(token, REDIRECT_STDIN) == 0) {
      fname = strtok(NULL, LINE_TOKEN_SEP);

      if (fname == NULL) {
        fprintf(stderr, "[ERROR] Parse error of line, no file specified for" 
          "redirection\n");
        exit(1);
      }

      process.in = open(fname, O_RDONLY);
      if (process.in == -1) {
        fprintf(stderr, "[ERROR] Could not open %s\n", fname);
        exit(1);
      }
    } else {
      process.args[argc++] = token;
    }

    // we need at LEAST one extra space for the NULL terminating argv
    if (command_capacity == argc) { 
      victim = process.args;

      command_capacity *= 2;
      process.args = malloc(sizeof(char*) * command_capacity);
      if (process.args == NULL) {
        fprintf(stderr, "[ERROR] Out of memory!");
        exit(1);
      }

      for (size_t i = 0; i < argc; ++i) {
        process.args[i] = victim[i];
      }

      free(victim);
    }
  } while((token = strtok(NULL, LINE_TOKEN_SEP)) != NULL);

  process.args[argc] = NULL; // we do resizing to account for this

  return process;
}