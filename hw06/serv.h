#pragma once

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#define DEFAULT_SERVER_NAME "server"
#define DEFAULT_PORT "9000"
#define NUM_WAITING_CONNECTIONS 10

int sock_fd, connfd;

int main(int, char**);
void request_to_quit(int, siginfo_t*, void*);