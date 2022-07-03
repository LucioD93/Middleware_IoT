#ifndef MIDDLEWARE_WORKER_UTILS_H
#define MIDDLEWARE_WORKER_UTILS_H

#include <signal.h>
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
#include <time.h>
#include <getopt.h>
#include <semaphore.h>
#include "constants.h"
#include "metadata_utils.h"
#include "socket_utils.h"
#include "threading_utils.h"

typedef struct worker_arg {
    int *socket;
    int *tasks_tracker;
} worker_args;

void worker(char master_server_address[16], int gpu);

#endif //MIDDLEWARE_WORKER_UTILS_H
