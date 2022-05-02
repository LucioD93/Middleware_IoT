#include "master_utils.h"


int main(void) {
    pthread_t worker_connections_thread, client_connections_thread;
    pthread_create(&worker_connections_thread, NULL, (void *(*)(void *)) master_worker_server, NULL);
    pthread_create(&client_connections_thread, NULL, (void *(*)(void *)) client_connections_server, NULL);
    pthread_join(client_connections_thread, NULL);
    pthread_join(worker_connections_thread, NULL);
}
