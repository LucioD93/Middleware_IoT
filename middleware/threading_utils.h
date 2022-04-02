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

#define BUFFERSIZE 4096
#define SOCKETERROR (-1)
#define SERVERBACKLOG 10
#define THREAD_POOL_SIZE 2


_Noreturn void master_server();

void worker(Metadata);

#endif //MIDDLEWARE_THREADING_UTILS_H
