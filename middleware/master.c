#include "master_utils.h"


int main(void) {
    pthread_t worker_connections_thread;
    pthread_create(&worker_connections_thread, NULL, (void *(*)(void *)) master_server, NULL);
    pthread_join(worker_connections_thread, NULL);
}
