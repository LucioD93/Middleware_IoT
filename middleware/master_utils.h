#ifndef MIDDLEWARE_MASTER_UTILS_H
#define MIDDLEWARE_MASTER_UTILS_H

#include "threading_utils.h"

_Noreturn void master_worker_server(void *arg);
_Noreturn void client_connections_server(void *arg);

#endif //MIDDLEWARE_MASTER_UTILS_H
