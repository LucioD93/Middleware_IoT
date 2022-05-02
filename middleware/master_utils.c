#include "master_utils.h"

pthread_t workers_thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t worker_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t worker_pool_condition_var = PTHREAD_COND_INITIALIZER;

pthread_t clients_thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t client_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t client_pool_condition_var = PTHREAD_COND_INITIALIZER;


_Noreturn void * handle_worker_connection(void* p_worker_socket) {
    int worker_socket = *((int*)p_worker_socket);
    free(p_worker_socket);
    printf("Got a connection from %d\n", worker_socket);
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t message_size;
    char uuid[UUID_STR_LEN];

    while(true) {
        memset(&buffer, 0, BUFFERSIZE);
        bytes_read = 0;
        message_size = 0;
        // read the worker's message
        while((bytes_read = read(worker_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
            message_size += bytes_read;
            if(message_size > BUFFERSIZE - 1 || buffer[message_size - 1] == 0) break;
        }
        if(strlen(buffer) == 0) {
            // Bad message. Close connection
            remove_from_list(uuid);
            check((close(worker_socket)), "Worker socket closing failed!");
            show_list();
        }
//        check(bytes_read, "Receiving failed!");
        buffer[message_size - 1] = 0; // null terminate
        Metadata worker_metadata = str_to_metadata(buffer);
        if(worker_metadata.resources.cpu < 10 && worker_metadata.resources.ram < 10 && worker_metadata.resources.gpu < 10) {
            strcpy(uuid, worker_metadata.uuid);

            pthread_mutex_lock(&worker_pool_mutex);
            add_to_list(worker_metadata);
            pthread_mutex_unlock(&worker_pool_mutex);

            show_list();
        }

        fflush(stdout);
    }
}


void * handle_client_connection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    printf("Got a connection from %d\n", client_socket);
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t message_size;memset(&buffer, 0, BUFFERSIZE);
    bytes_read = 0;
    message_size = 0;
    // read the client's message
    while((bytes_read = read(client_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
        message_size += bytes_read;
        if(message_size > BUFFERSIZE - 1 || buffer[message_size - 1] == 0) break;
    }
    buffer[message_size - 1] = 0; // null terminate
    printf("Received from client \n|%s|\n", buffer);
    close(client_socket);
    fflush(stdout);
}


_Noreturn void * worker_connection_thread_function(void *arg) {
    while (true) {
        pthread_mutex_lock(&worker_pool_mutex);
        pthread_cond_wait(&worker_pool_condition_var, &worker_pool_mutex);
        int *p_worker = dequeue_worker_connection();
        pthread_mutex_unlock(&worker_pool_mutex);
        if (p_worker != NULL) {
            // There is a connection
            printf("WORKER CONNECTED\n");
            handle_worker_connection(p_worker);
        }
    }
}


_Noreturn void * client_connection_thread_function(void *arg) {
    while (true) {
        pthread_mutex_lock(&client_pool_mutex);
        pthread_cond_wait(&client_pool_condition_var, &client_pool_mutex);
        int *p_client = dequeue_client_connection();
        pthread_mutex_unlock(&client_pool_mutex);
        if (p_client != NULL) {
            // There is a connection
            printf("CLIENT CONNECTED\n");
            handle_client_connection(p_client);
        }
    }
}


_Noreturn void master_worker_server(void *arg) {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&workers_thread_pool[i], NULL, worker_connection_thread_function, NULL);
    }

    check(
            (server_socket = socket(AF_INET, SOCK_STREAM, 0)),
            "Failed to create socket!"
    );

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(WORKERS_PORT);

    check(
            bind(
                    server_socket,
                    (SA*) &server_addr,
                    sizeof(server_addr)
            ),
            "Bind failed!"
    );

    check(
            listen(server_socket, SERVERBACKLOG),
            "Listen failed!"
    );

    while(true) {
        printf("Waiting worker connections...\n");
        // wait for and accept an incoming connection
        addr_size = sizeof(SA_IN);

        check(
                (client_socket = accept(
                        server_socket,
                        (SA*)&client_addr,
                        (socklen_t*) &addr_size
                )),
                "Accept failed!"
        );

        // Queue connection so that a worker thread can grab it
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;
        printf("ACCEPTED WORKER CONNECTION\n");

        // Prevent race condition
        pthread_mutex_lock(&worker_pool_mutex);
        enqueue_worker_connection(pclient);
        pthread_cond_signal(&worker_pool_condition_var);
        pthread_mutex_unlock(&worker_pool_mutex);
    }
}


_Noreturn void client_connections_server(void *args) {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&clients_thread_pool[i], NULL, client_connection_thread_function, NULL);
    }

    check(
            (server_socket = socket(AF_INET, SOCK_STREAM, 0)),
            "Failed to create socket!"
    );

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(CLIENTS_PORT);

    check(
            bind(
                    server_socket,
                    (SA*) &server_addr,
                    sizeof(server_addr)
            ),
            "Bind failed!"
    );

    check(
            listen(server_socket, SERVERBACKLOG),
            "Listen failed!"
    );

    while(true) {
        printf("Waiting client connections...\n");
        // wait for and accept an incoming connection
        addr_size = sizeof(SA_IN);

        check(
                (client_socket = accept(
                        server_socket,
                        (SA*)&client_addr,
                        (socklen_t*) &addr_size
                )),
                "Accept failed!"
        );

        // Queue connection so that a client thread can grab it
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;
        printf("ACCEPTED CLIENT CONNECTION\n");

        // Prevent race condition
        pthread_mutex_lock(&client_pool_mutex);
        enqueue_client_connection(pclient);
        pthread_cond_signal(&client_pool_condition_var);
        pthread_mutex_unlock(&client_pool_mutex);
    }
}
