#pragma once

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define CONN_BUF_SIZE 512

typedef struct {
  int conn_fd;
  pthread_t conn_tid;
  char* uname;
} Connection;

typedef struct connnode{
  Connection* conn;
  struct connnode* next;
} ConnNode;

typedef struct {
  ConnNode* conns;
  pthread_mutex_t mut;
} ConnManager;


ConnManager* createConnManager();
void addConnection(ConnManager*, Connection*);
void removeConnection(ConnManager*, Connection*);
void sendAllUsernames(ConnManager*, int);