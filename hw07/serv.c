#include "serv.h"

int main(int argc, char** argv) {
  int sock_fd;
  pthread_t distributorid;
  Messages* messages;
  ConnManager* c_man;
  Manager manager;
  void* retval;

  if (argc > 1) { // we have a port
    sock_fd = createServer(argv[1]);
  } else {
    sock_fd = createServer(DEFAULT_PORT);
  }

  if (sock_fd < 0) exit(1);

  messages = createMessageQueue();
  if (messages == NULL) exit(1);

  c_man = createConnManager();
  if (c_man == NULL) exit(1);

  manager.c_man = c_man;
  manager.messages = messages;

  pthread_create(&distributorid, NULL, message_distributor, &manager);

  listenAndHandle(sock_fd, messages, c_man);

  pthread_join(distributorid, &retval);
  // if the message distributor dies we die too
}

int createServer(char* service) {
  int err, sock_fd;
  struct addrinfo hints, *result;
  
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    fprintf(stderr, "Error opening socket\n");
    return -1;
  } // errored to the point of death

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  err = getaddrinfo(NULL, service, &hints, &result);
  if (err != 0) {
    fprintf(stderr, "getaddrinfo err: %s\n", gai_strerror(err));
    return -1;
  }

  if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
    fprintf(stderr, "Could not bind to port %s\n", service);
    return -1;
  }

  return sock_fd;
}

void listenAndHandle(int sock_fd, Messages* messages, ConnManager* c_man) {
  int conn_fd, err;
  Manager* manager;
  Context* ctx;

  if (listen(sock_fd, NUM_WAITING_CONNECTIONS)  != 0) {
    fprintf(stderr, "[server] Could not listen\n");
    exit(1);
  }

  for(;;) {
    conn_fd = accept(sock_fd, NULL, NULL);
    
    if (conn_fd == -1) {
      fprintf(stderr, "[server] Socket fd errored out\n");
      break;
    }

    while((ctx = malloc(sizeof(Context))) == NULL); // this MUST not err
    ctx->conn.conn_fd = conn_fd;

    ctx->c_man = c_man;
    ctx->messages = messages;

    pthread_create(&ctx->conn.conn_tid, NULL, handle_connection, ctx);
    pthread_detach(ctx->conn.conn_tid);
  }
}

void* handle_connection(void* arg) {
  int err, nbytes, keep_listening, conn_fd;  
  Context* ctx = arg;
  fd_set connections;
  char buf[SERV_BUF_SIZE + 1], msg[MAX_MSG_SIZE + 1];

  puts("[server] New connection");
  keep_listening = 1;
  conn_fd = ctx->conn.conn_fd;

  addConnection(ctx->c_man, &ctx->conn);

  puts("[server] Waiting for client greeting...");

  nbytes = read(conn_fd, buf, SERV_BUF_SIZE);
  if (nbytes == -1) { // if error
    fprintf(stderr, "[server] Error reading from conn_fd\n");
    keep_listening = 0;
  } else if (nbytes == 0) { // if EOF
    puts("[server] Client did not send greeting.");
    keep_listening = 0;
  } else {
    buf[nbytes] = 0; // we want to ignore newlines.

    if (strstr(buf, CLIENT_GREETING_PREFIX) != buf) {
      puts("[server] Client sent incorrect greeting. Bad protocol. Dropping connection.");
      keep_listening = 0;
    } else {
      ctx->conn.uname = strdup(&buf[strlen(CLIENT_GREETING_PREFIX)]);
      
      snprintf(buf, SERV_BUF_SIZE, "User [%s] has joined the chat.", ctx->conn.uname);
      addMessage(ctx->messages, buf, &ctx->conn);
    }
  }

  sendAllUsernames(ctx->c_man, conn_fd);

  for(;keep_listening;) {
    nbytes = read(conn_fd, buf, SERV_BUF_SIZE);

    if (nbytes == 0) { // if EOF from client
      puts("[server] Client disconnected");
      break;
    }

    buf[nbytes] = 0;

    snprintf(msg, MAX_MSG_SIZE, "[%s] %s", ctx->conn.uname, buf);

    addMessage(ctx->messages, msg, &ctx->conn);
  }

  removeConnection(ctx->c_man, &ctx->conn);

  snprintf(buf, SERV_BUF_SIZE, "User [%s] has disconnected.", ctx->conn.uname);
  addMessage(ctx->messages, buf, &ctx->conn);

  free(ctx->conn.uname); // we strdup'd this
  free(ctx);

  return NULL;
}

void* message_distributor(void* arg) {
  Manager* manager = arg;
  Message* message;

  for (;;) {
    message = popMessage(manager->messages);
    printf("[distributor] Sending message: <%.15s>\n", message->msg);
    distributeMessage(manager->c_man, message);

    free(message->msg);
    free(message);
  }

  return NULL;
}

