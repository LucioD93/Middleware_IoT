#include "worker_utils.h"

void term() {
    printf("Exit with ctrl + Z!\n");
}

int main(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGINT, &action, NULL);

    worker();
}
