#include "threading_utils.h"

int check(int exp, const char *msg) {
    if (exp == SOCKET_ERROR) {
        perror(msg);
        exit(1);
    }
    return exp;
}
