#pragma once

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#define DEFAULT_SERVER "0.0.0.0"
#define DEFAULT_PORT  "9000"
#define DEFAULT_UNAME "cli"
#define CLIENT_GREETING "HELLO MY USERNAME IS:%s"

#define CLI_BUF_SIZE 511

int main(int, char**);
int connectToServer(char*, char*);
void talkToServer(int, char*);