#include "worker_utils.h"

void term() {
    printf("Caught!\n");
    keep_running = 0;
}

int main(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGINT, &action, NULL);

    worker();
}
