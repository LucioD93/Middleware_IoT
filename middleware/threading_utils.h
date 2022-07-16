#ifndef MIDDLEWARE_THREADING_UTILS_H
#define MIDDLEWARE_THREADING_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include "constants.h"
#include "queue.h"
#include "metadata_utils.h"

#define BUFFER_SIZE 4096
#define SOCKET_ERROR (-1)
#define SERVER_BACKLOG 5000

int check(int exp, const char *msg);

bool isValidIpAddress(char *ipAddress);

#endif //MIDDLEWARE_THREADING_UTILS_H
