#include "cli.h"

int main(int argc, char** argv) {
  char *node, *service, *uname;
  int sock_fd;

  if (argc > 1) {
    node = argv[1];
  } else {
    node = DEFAULT_SERVER;
  }

  if (argc > 2) {
    service = argv[2];
  } else {
    service = DEFAULT_PORT;
  }

  if (argc > 3) {
    uname = argv[3];
  } else {
    uname = DEFAULT_UNAME;
  }

  sock_fd = connectToServer(node, service);
  talkToServer(sock_fd, uname);
}

int connectToServer(char* node, char* service) {
  int sock_fd, err;
  struct addrinfo hints, *results;

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  err = getaddrinfo(node, service, &hints, &results);

  if (err != 0) {
    fprintf(stderr, "getaddrinfo err: %s\n", gai_strerror(err));
    exit(1);
  }

  err = connect(sock_fd, results->ai_addr, results->ai_addrlen);
  if (err == -1) {
    fprintf(stderr, "Could not connect to server\n");
    exit(errno);
  }


  return sock_fd;
}

void talkToServer(int sock_fd, char* uname) {
  fd_set connection;
  int nread, err;
  char buf[CLI_BUF_SIZE + 1];

  dprintf(sock_fd, CLIENT_GREETING, uname);

  for(;;) {
    printf("[%s] ", uname);
    fflush(stdout);

    FD_ZERO(&connection);
    FD_SET(STDIN_FILENO, &connection);
    FD_SET(sock_fd, &connection);

    select(sock_fd + 1, &connection, NULL, NULL, NULL);

    if (FD_ISSET(sock_fd, &connection)) {
      nread = read(sock_fd, buf, CLI_BUF_SIZE);
      if (nread <= 0) break;
      buf[nread] = 0;

      printf("\33[2K\r%s\n", buf);
    }

    if (FD_ISSET(STDIN_FILENO, &connection)) {
      nread = read(STDIN_FILENO, buf, CLI_BUF_SIZE);
      if (nread <= 0) break;
      buf[nread] = 0;
      if (buf[nread - 1] == '\n') buf[nread - 1] = 0;
      err = dprintf(sock_fd, "%s", buf);
      if (err == -1) break;
    }
  }

  shutdown(sock_fd, SHUT_RDWR);
  close(sock_fd);
}