#include "master_utils.h"


int main(int argc, char *argv[]) {

    int gpu = 1;
    int option;
    while((option = getopt(argc, argv, "g:")) != -1) {
        switch(option) {
        case 'g': //
            gpu = atoi(optarg);
            break;
        }
    }

    pthread_t worker_connections_thread, client_connections_thread;
    pthread_create(&worker_connections_thread, NULL, (void *(*)(void *)) worker_connections_server, NULL);
    pthread_create(&client_connections_thread, NULL, (void *(*)(void *)) client_connections_server, NULL);
    sleep_for_milliseconds(50);
    char master_server_address[16] = DEFAULT_MASTER_SERVER_ADDRESS;
    worker(master_server_address, gpu);
    pthread_join(client_connections_thread, NULL);
    pthread_join(worker_connections_thread, NULL);
}
