#include "messages.h"

Messages* createMessageQueue(size_t capacity) {
  int err;
  Messages* messages;

  messages = malloc(sizeof(Messages));
  if (messages == NULL) {
    fprintf(stderr, "Could not create messages object\n");
    return NULL;
  }

  messages->queue = malloc(sizeof(Message*) * capacity);
  if (messages->queue == NULL) {
    fprintf(stderr, "Could not create message queue object\n");
    free(messages);
    return NULL;
  }
  messages->queue_capacity = capacity;
  messages->num_messages = 0; // starts with 0 messages

  err = pthread_mutex_init(&messages->mut, NULL);
  if (err != 0) {
    fprintf(stderr, "Could not create mutex for message queue\n"); 
    free(messages->queue);
    free(messages);
    return NULL;
  }

  err = pthread_cond_init(&messages->has_items, NULL);
  if (err != 0) {
    fprintf(stderr, "Could not create condition variable for message queue\n");
    free(messages->queue);
    free(messages);
    return NULL;
  }

  err = pthread_cond_init(&messages->has_space, NULL);
  if (err != 0) {
    fprintf(stderr, "Could not create condition variable for message queue\n");
    free(messages->queue);
    free(messages);
    return NULL;
  }

  return messages;
}

void cleanupMessagesQueue(Messages* messages) {
  pthread_mutex_destroy(&messages->mut);
  pthread_cond_destroy(&messages->has_items);
  pthread_cond_destroy(&messages->has_space);

  for (size_t i = 0; i < messages->num_messages; ++i) {
    free(messages->queue[i]);
  }

  free(messages->queue);
  free(messages);
}

int addMessage(Messages* messages, char* message, Connection* conn) {
  Message* new_message;

  new_message = malloc(sizeof(Message));
  if (new_message == NULL) {
    fprintf(stderr, "Malloc failed\n");
    return -1;
  }

  new_message->msg = strdup(message);
  new_message->sender = conn;

  if (new_message == NULL) {
    fprintf(stderr, "Could not add message to queue. Out of memory.\n");
    return -1;
  }

  pthread_mutex_lock(&messages->mut);
  while(messages->num_messages == messages->queue_capacity) {
    pthread_cond_wait(&messages->has_space, &messages->mut);
  }

  messages->queue[messages->num_messages++] = new_message;

  pthread_mutex_unlock(&messages->mut);
  pthread_cond_broadcast(&messages->has_items);
  return 0;
}

// It is the program's job to free the message that this returns.
Message* popMessage(Messages* messages) {
  Message* message;

  pthread_mutex_lock(&messages->mut);
  while(messages->num_messages == 0) {
    pthread_cond_wait(&messages->has_items, &messages->mut);
  }

  message = messages->queue[--messages->num_messages];

  pthread_mutex_unlock(&messages->mut);
  pthread_cond_broadcast(&messages->has_space);

  return message;
}

void distributeMessage(ConnManager* c_man, Message* message) {
  ConnNode* curr;

  if (c_man->conns == NULL) return; // scream into the void
  curr = c_man->conns;

  while(curr) {
    if (curr->conn->conn_fd == message->sender->conn_fd) {
      curr = curr->next;
      continue;
    }
    dprintf(curr->conn->conn_fd, "%s", message->msg);
    curr = curr->next;
  }
}