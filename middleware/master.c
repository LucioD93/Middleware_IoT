#include "master_utils.h"

int main(int argc, char *argv[])
{

    int gpu = 1;
    bool create_local_worker = false;
    int option;
    while((option = getopt(argc, argv, "g:w")) != -1) {
        switch(option) {
        case 'g': //
            gpu = atoi(optarg);
            break;
        case 'w':
            create_local_worker = true;
            break;
        }
    }

    pthread_t worker_connections_thread, client_connections_thread;
    pthread_create(&worker_connections_thread, NULL, (void *(*)(void *))worker_connections_server, NULL);
    pthread_create(&client_connections_thread, NULL, (void *(*)(void *))client_connections_server, NULL);

    if (create_local_worker) {
        sleep_for_milliseconds(50);
        char master_server_address[16] = DEFAULT_MASTER_SERVER_ADDRESS;
        worker(master_server_address, gpu);
    }

    pthread_join(client_connections_thread, NULL);
    pthread_join(worker_connections_thread, NULL);
}
