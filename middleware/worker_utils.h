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
#include "constants.h"
#include "metadata_utils.h"
#include "threading_utils.h"


void worker();

#endif //MIDDLEWARE_WORKER_UTILS_H
