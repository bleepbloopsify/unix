#include "cli.h"

int main(int argc, char** argv) {
  char *addr, *port, *uname;
  struct addrinfo hints, *result;
  int err, bytes_read;
  char buf[256];
  fd_set connections;
  struct sigaction action;

  if (argc > 1) {
    addr = argv[1];
  } else {
    addr = DEFAULT_ADDR;
  }

  if (argc > 2) {
    port = argv[2];
  } else {
    port = DEFAULT_PORT;
  }

  if (argc > 3) {
    uname = argv[3];
  } else {
    uname = DEFAULT_UNAME;
  }

  action.sa_sigaction = request_to_quit;
  sigemptyset(&action.sa_mask);

  sigaction(SIGINT, &action, NULL);
  sigaction(SIGABRT, &action, NULL);

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    fprintf(stderr, "Error creating socket\n");
    exit(errno);
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  err = getaddrinfo(addr, port, &hints, &result);

  if (err != 0) {
    fprintf(stderr, "getaddrinfo err: %s\n", gai_strerror(err));
    exit(1);
  }

  err = connect(sock_fd, result->ai_addr, result->ai_addrlen);
  if (err == -1) {
    fprintf(stderr, "Could not connect to server\n");
    exit(errno);
  }

  FD_ZERO(&connections);

  puts("Waiting for greeting from server...");
  bytes_read = read(sock_fd, buf, 255);
  if (bytes_read == -1) {
    fprintf(stderr, "[Error] Did not receive greeting from server");
    exit(1);
  }
  buf[bytes_read] = 0;
  if (bytes_read > 0) {
    if (buf[bytes_read - 1] == '\n') buf[bytes_read - 1] = 0;
  }
  puts(buf);

  puts("Sending greeting to server...");

  dprintf(sock_fd, "[%s] Hello from client\n", uname);

  for(;;) {
    printf("[%s] ", uname);
    fflush(stdout);


    FD_SET(sock_fd, &connections);
    FD_SET(STDIN_FILENO, &connections);

    select(sock_fd + 1, &connections, NULL, NULL, NULL);

    if (FD_ISSET(sock_fd, &connections)) {
      bytes_read = read(sock_fd, buf, 255);
      if (bytes_read <= 0) break;
      buf[bytes_read] = 0;
      if (bytes_read > 0) {
        if (buf[bytes_read - 1] == '\n') buf[bytes_read - 1] = 0;
      }

      printf("\33[2K\r%s\n", buf);
      fflush(stdout);
    }

    if (FD_ISSET(STDIN_FILENO, &connections)) {
      bytes_read = read(STDIN_FILENO, buf, 255);
      if (bytes_read <= 0) break;
      buf[bytes_read] = 0;
      err = dprintf(sock_fd, "[%s] %s", uname, buf);
      if (err == -1) break;
    }
  }

  shutdown(sock_fd, SHUT_RDWR);
  close(sock_fd);


  puts("Closed connection");

  return 0;
}

void request_to_quit(int sig, siginfo_t* info, void* ucontext) {
  shutdown(sock_fd, SHUT_RDWR);
  close(sock_fd);

  puts("Closed connection");

  exit(0);
}