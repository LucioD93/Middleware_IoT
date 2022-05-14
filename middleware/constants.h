#ifndef MIDDLEWARE_CONSTANTS_H
#define MIDDLEWARE_CONSTANTS_H

// Networking constants
#define WORKERS_PORT 8989
#define CLIENTS_PORT 8991
#define RESPONSES_PORT 8995
#define MAXLINE 4096
#define MASTERSERVERADDRESS "127.0.0.1"
#define WORKERSYNCTIMER 10


// Requests types
#define IMAGEPROCESSINGREQUEST "1"
#define WEBREQUEST "2"
#define WORDPROCESSINGREQUEST "3"
#define SYNCHRONIZATIONREQUEST "4"
#define IMAGELOCATIONINGREQUEST "5"
#define IPLOCATIONINGREQUEST "6"


typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// Algorithm constants
static const float beta = 0.04;

#endif //MIDDLEWARE_CONSTANTS_H
