#include "connmanager.h"


ConnManager* createConnManager() {
  int err;
  ConnManager* c_man;

  c_man = malloc(sizeof(ConnManager));
  if (c_man == NULL) {
    fprintf(stderr, "Could not create connection manager, out of memory\n");
    return NULL;
  }

  c_man->conns = NULL;

  err = pthread_mutex_init(&c_man->mut, NULL);
  if (err != 0) {
    fprintf(stderr, "Could not create mutex for connection manager\n");
    free(c_man);
    return NULL;
  }

  return c_man;
}

void addConnection(ConnManager* c_man, Connection* conn) {
  ConnNode* node;

  pthread_mutex_lock(&c_man->mut);

  node = malloc(sizeof(ConnNode));

  node->conn = conn;

  node->next = c_man->conns;
  c_man->conns = node;

  pthread_mutex_unlock(&c_man->mut);
}

void removeConnection(ConnManager* c_man, Connection* conn) {
  ConnNode* curr, *victim;
  
  pthread_mutex_lock(&c_man->mut);

  curr = c_man->conns;
  if (curr->conn->conn_fd == conn->conn_fd) {
    // HEAD is the offender >(
    c_man->conns = curr->next;
    free(curr);
    pthread_mutex_unlock(&c_man->mut);
    return;
  }

  while(curr->next != NULL) {
    if (curr->next->conn->conn_fd == conn->conn_fd) {
      // we found the offender
      victim = curr->next;
      curr->next = victim->next;
      free(victim);
      break;
    }

    curr = curr->next;
  }

  pthread_mutex_unlock(&c_man->mut);
}

void sendAllUsernames(ConnManager* c_man, int conn_fd) {
  char buf[CONN_BUF_SIZE];
  ConnNode* curr;

  if (c_man->conns == NULL) {
    dprintf(conn_fd, "No users currently online.\n");
  } else {
    curr = c_man->conns;
    while(curr) {
      if (curr->conn->conn_fd == conn_fd)  {
        curr = curr->next;
        continue;
      }
      dprintf(conn_fd, "%s is online.", curr->conn->uname);
      curr = curr->next;
    }
  }
}