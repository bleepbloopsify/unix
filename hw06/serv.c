#include "serv.h"

int main(int argc, char** argv) {
  int err;
  size_t bytes_read;
  struct addrinfo hints, *result;
  fd_set sockets, connection;
  struct sigaction action;

  char buf[256];

  char* port, *uname;

  if (argc == 1) {
    puts("Usage: serv <port> <uname>");
  } // this is not a failure condition

  if (argc > 1) {
    port = argv[1];
  } else {
    port = DEFAULT_PORT;
  }

  if (argc > 2) {
    uname = argv[2];
  } else {
    uname = DEFAULT_SERVER_NAME;
  }

  action.sa_sigaction = request_to_quit;
  sigemptyset(&action.sa_mask);

  sigaction(SIGINT, &action, NULL);
  sigaction(SIGABRT, &action, NULL);

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    fprintf(stderr, "Error opening socket\n");
    exit(errno);
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  err = getaddrinfo(NULL, port, &hints, &result);
  if (err != 0) {
    fprintf(stderr, "getaddrinfo err: %s\n", gai_strerror(err));
    exit(1);
  }

  if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
    fprintf(stderr, "Could not bind\n");
    exit(1);
  }

  if (listen(sock_fd, NUM_WAITING_CONNECTIONS) != 0) {
    fprintf(stderr, "Could not listen\n");
    exit(1);
  }

  FD_ZERO(&sockets);

  printf("Listening on port %s\n", port);

  for (;;) {
    FD_SET(sock_fd, &sockets);

    select(sock_fd + 1, &sockets, NULL, NULL, NULL);

    puts("\naccepted connection!");

    connfd = accept(sock_fd, result->ai_addr, &result->ai_addrlen);

    FD_ZERO(&connection);

    puts("Sending greeting to client...");
    dprintf(connfd, "[%s] Hello from server\n", uname);

    puts("Waiting for greeting from client...");
    bytes_read = read(connfd, buf, 255);
    if (bytes_read == -1) {
      fprintf(stderr, "[Error] Did not receive greeting from client");
      exit(1);
    }
    buf[bytes_read] = 0;
    if (bytes_read > 0) {
      if (buf[bytes_read - 1] == '\n') buf[bytes_read - 1] = 0;
    }
    puts(buf);

    for(;;) {
      FD_SET(connfd, &connection);
      FD_SET(STDIN_FILENO, &connection);

      printf("[%s] ", uname);
      fflush(stdout);

      select(connfd + 1, &connection, NULL, NULL, NULL);

      if (FD_ISSET(connfd, &connection)) {
        bytes_read = read(connfd, buf, 255);
        if (bytes_read <= 0) break;
        buf[bytes_read] = 0;

        if (bytes_read > 0) {
          if (buf[bytes_read - 1] == '\n') buf[bytes_read - 1] = 0;
        }

        printf("\33[2K\r%s\n", buf);
        fflush(stdout);
      }

      if (FD_ISSET(STDIN_FILENO, &connection)) {
        bytes_read = read(STDIN_FILENO, buf, 255);
        buf[bytes_read] = 0;
        err = dprintf(connfd, "[%s] %s", uname, buf);
        if (err == -1) break;
      }
    }

    close(connfd);

    puts("\nConnection closed");
  }
  close(sock_fd);

  puts("Server shutdown");

  return 0;
}

void request_to_quit(int sig, siginfo_t* info, void* ucontext) {
  close(connfd);
  shutdown(sock_fd, SHUT_RDWR);
  close(sock_fd);
 
  puts("Server shutdown");

  exit(0);
}

