#ifndef MIDDLEWARE_MASTER_UTILS_H
#define MIDDLEWARE_MASTER_UTILS_H

#include "threading_utils.h"
#include "worker_utils.h"
#include <netinet/sctp.h>

_Noreturn void worker_connections_server();
_Noreturn void client_connections_server();

#endif //MIDDLEWARE_MASTER_UTILS_H
