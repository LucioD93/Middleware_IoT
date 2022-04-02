#ifndef MIDDLEWARE_CONSTANTS_H
#define MIDDLEWARE_CONSTANTS_H

// Networking constants
#define SERVERPORT 8989
#define MAXLINE 4096
#define SERVERADDRESS "127.0.0.1"

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// Algorithm constants
static const float beta = 0.04;

#endif //MIDDLEWARE_CONSTANTS_H
