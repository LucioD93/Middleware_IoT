#include "master_utils.h"


int main(void) {
    pthread_t worker_connections_thread, client_connections_thread, local_worker_thread;
    pthread_create(&worker_connections_thread, NULL, (void *(*)(void *)) worker_connections_server, NULL);
    pthread_create(&client_connections_thread, NULL, (void *(*)(void *)) client_connections_server, NULL);
    sleep_for_milliseconds(500);
    char master_server_address[16] = DEFAULT_MASTER_SERVER_ADDRESS;
    worker(master_server_address, 1);
    pthread_join(client_connections_thread, NULL);
    pthread_join(worker_connections_thread, NULL);
}
