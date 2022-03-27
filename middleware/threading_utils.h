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
#include "queue.h"

#define SERVERPORT 8989
#define BUFFERSIZE 4096
#define SOCKETERROR (-1)
#define SERVERBACKLOG 10
#define THREAD_POOL_SIZE 2

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;


void master_server();

#endif //MIDDLEWARE_THREADING_UTILS_H
