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

#define DEFAULT_ADDR "localhost"
#define DEFAULT_PORT "9000"
#define DEFAULT_UNAME "cli"

int sock_fd;

int main(int, char**);
void request_to_quit(int, siginfo_t*, void*);