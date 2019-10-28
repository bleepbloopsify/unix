#pragma once

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "connmanager.h"

typedef struct {
  char* msg;
  Connection* sender;
} Message;

typedef struct {
  size_t num_messages;
  size_t queue_capacity;
  Message** queue;
  pthread_mutex_t mut;
  pthread_cond_t has_items;
  pthread_cond_t has_space;
} Messages;

Messages* createMessageQueue();
void cleanupMessagesQueue(Messages*);
int addMessage(Messages* messages, char* message, Connection* conn);
Message* popMessage(Messages* messages);

void distributeMessage(ConnManager*, Message*);
