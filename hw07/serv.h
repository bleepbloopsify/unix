#pragma once

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>

#include "messages.h"
#include "connmanager.h"

#define DEFAULT_PORT "9000"
#define NUM_WAITING_CONNECTIONS 10
#define SERV_BUF_SIZE 511
#define MAX_MSG_SIZE 255
#define CLIENT_GREETING_PREFIX "HELLO MY USERNAME IS:"

typedef struct {
  ConnManager* c_man;
  Messages* messages;
} Manager;

typedef struct {
  Connection conn;

  ConnManager* c_man;
  Messages* messages;
} Context;

int main(int, char**);
int createServer(char*);
void listenAndHandle(int, Messages*, ConnManager*);
void* handle_connection(void*);
void* message_distributor(void*);