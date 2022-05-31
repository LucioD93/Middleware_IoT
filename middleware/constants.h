#ifndef MIDDLEWARE_CONSTANTS_H
#define MIDDLEWARE_CONSTANTS_H

// Networking constants
#define WORKERS_PORT 8989
#define CLIENTS_PORT 8991
#define RESPONSES_PORT 8995
#define MAX_LINE 4096
#define MASTER_SERVER_ADDRESS "127.0.0.1"
#define WORKER_SYNC_TIMER 10


// Requests types
#define IMAGE_PROCESSING_REQUEST 1
#define WEB_REQUEST 2
#define WORD_PROCESSING_REQUEST 3
#define SYNCHRONIZATION_REQUEST 4
#define IMAGE_LOCATION_REQUEST 5
#define IP_LOCATION_REQUEST 6


typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// Algorithm constants
static const float beta = 0.04;

#endif //MIDDLEWARE_CONSTANTS_H
