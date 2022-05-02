#ifndef MIDDLEWARE_CLIENT_UTILS_H
#define MIDDLEWARE_CLIENT_UTILS_H

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
#include "constants.h"
#include "threading_utils.h"

_Noreturn void client_thread_thread();

#endif //MIDDLEWARE_CLIENT_UTILS_H
