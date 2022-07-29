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
#include <getopt.h>
#include <netinet/sctp.h>
#include <float.h>
#include "constants.h"
#include "socket_utils.h"
#include "threading_utils.h"
#include "metadata_utils.h"

void client_function(
    int request_type,
    char master_server_address[16]
);

#endif //MIDDLEWARE_CLIENT_UTILS_H
