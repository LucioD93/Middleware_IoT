#include "master_utils.h"

pthread_t workers_thread_pool[WORKERS_THREAD_POOL_SIZE];
pthread_mutex_t worker_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t worker_pool_condition_var = PTHREAD_COND_INITIALIZER;

pthread_t clients_thread_pool[CLIENT_THREAD_POOL_SIZE];
pthread_mutex_t client_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t client_pool_condition_var = PTHREAD_COND_INITIALIZER;
sem_t sem_client_queue;

pthread_mutex_t worker_selection_mutex = PTHREAD_MUTEX_INITIALIZER;

_Noreturn void * handle_worker_connection(void* p_worker_socket) {
    int worker_socket = *((int*)p_worker_socket);
    free(p_worker_socket);
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t message_size;
    char uuid[UUID_STR_LEN];

    while(true) {
        memset(&buffer, 0, BUFFER_SIZE);
        message_size = 0;
        // read the worker's message
        while(
            (bytes_read = read(worker_socket, buffer + message_size,sizeof(buffer) - message_size)) > 0
        ) {
            message_size += bytes_read;
            if(message_size > BUFFER_SIZE - 1 || buffer[message_size - 1] == 0) break;
        }
        // if(strlen(buffer) == 0) {
        //     // Bad message. Close connection
        //     check((close(worker_socket)), "Worker socket closing failed!");
        //     remove_from_list(uuid);
        // }
        buffer[message_size - 1] = 0; // null terminate
        Metadata worker_metadata = str_to_metadata(buffer);

        if(
            worker_metadata.resources.cpu <= 10 &&
            worker_metadata.resources.ram <= 10 &&
            worker_metadata.resources.gpu <= 10
        ) {
            strcpy(uuid, worker_metadata.uuid);

            pthread_mutex_lock(&worker_pool_mutex);
            add_to_list(worker_metadata, worker_socket);
            pthread_mutex_unlock(&worker_pool_mutex);
        }

        fflush(stdout);
    }
}


void *handle_client_connection(void* p_client_socket) {
    printf("Handling client\n");
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    char buffer[BUFFER_SIZE];
    size_t bytes_read, message_size;
    memset(&buffer, 0, BUFFER_SIZE);
    bytes_read = 0;
    message_size = 0;
    // read the client's message
    while((bytes_read = read(client_socket, buffer + message_size, 8 - message_size)) > 0) {
        message_size += bytes_read;
        if(message_size > 8 || buffer[message_size - 1] == 0) break;
    }
    buffer[message_size - 1] = 0; // null terminate

    int request_type, client_port_for_worker;
    sscanf(buffer, "%d-%d", &request_type, &client_port_for_worker);

    metadata_node *selected_worker = NULL;

    while(true) {
        pthread_mutex_lock(&worker_selection_mutex);
        selected_worker = select_worker(request_type);
        pthread_mutex_unlock(&worker_selection_mutex);
        if (selected_worker != NULL) break;
        sleep_for_milliseconds(15);
    }

    SA_IN client_address;
    int len;
    len = sizeof(client_address);
    check(
        getpeername(client_socket, (SA *)&client_address, &len),
        "Failed getpeername"
    );

    char request_representation[MAX_LINE];
    strcpy(request_representation, inet_ntoa(client_address.sin_addr));
    strcat(request_representation, " | ");
    strcat(request_representation, buffer);
    int sendbytes = 26;

    // Send client ip to worker socket
    check(
        (write(selected_worker->worker_socket, &request_representation, sendbytes) != sendbytes),
        "Socket write failed"
    );

    close(client_socket);
    fflush(stdout);
}


_Noreturn void * worker_connection_thread_function() {
    while (true) {
        pthread_mutex_lock(&worker_pool_mutex);
        pthread_cond_wait(&worker_pool_condition_var, &worker_pool_mutex);
        int *p_worker = dequeue_worker_connection();
        pthread_mutex_unlock(&worker_pool_mutex);
        if (p_worker != NULL) {
            // There is a connection
            handle_worker_connection(p_worker);
        }
    }
}


_Noreturn void * client_connection_thread_function() {
    while (true) {
        sem_wait(&sem_client_queue);
        pthread_mutex_lock(&client_pool_mutex);
        // pthread_cond_wait(&client_pool_condition_var, &client_pool_mutex);
        printf("Dequeued client connection\n");
        int *p_client = dequeue_client_connection();
        pthread_mutex_unlock(&client_pool_mutex);
        if (p_client != NULL) {
            // There is a connection
            handle_client_connection(p_client);
        }
    }
}


_Noreturn void worker_connections_server() {
    int server_socket, worker_socket, address_size;
    SA_IN server_address, worker_address;
    struct sctp_initmsg initmsg;

    // Create threads to handle connections
    for (int i = 0; i < WORKERS_THREAD_POOL_SIZE; i++) {
        pthread_create(
            &workers_thread_pool[i],
            NULL,
            worker_connection_thread_function,
            NULL
        );
    }

    check(
        (server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)),
        "Failed to create worker socket!"
    );
    
    memset (&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = 64;
    initmsg.sinit_max_instreams = 64;
    initmsg.sinit_max_attempts = 4;
    check(
        (setsockopt(server_socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg))),
        "master socket for worker setsockopt failed"
    );

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(WORKERS_PORT);

    check(
        bind(
            server_socket,
            (SA*) &server_address,
            sizeof(server_address)
        ),
        "Bind failed!"
    );

    check(
        listen(server_socket, SERVER_BACKLOG),
        "Listen failed!"
    );

    printf("Waiting for worker connections ...\n");

    while(true) {
        // wait for and accept an incoming connection
        address_size = sizeof(SA_IN);

        check(
            (
                worker_socket = accept(
                    server_socket,
                    (SA*)&worker_address,
                    (socklen_t*) &address_size
                )
            ),
            "Accept failed!"
        );

        // Queue connection so that a worker thread can grab it
        int *client_pointer = malloc(sizeof(int));
        *client_pointer = worker_socket;

        // Prevent race condition
        pthread_mutex_lock(&worker_pool_mutex);
        enqueue_worker_connection(client_pointer);
        pthread_cond_signal(&worker_pool_condition_var);
        pthread_mutex_unlock(&worker_pool_mutex);
    }
}


_Noreturn void client_connections_server() {
    sem_init(&sem_client_queue, 0, 0);
    int server_socket, client_socket, address_size;
    SA_IN server_address, client_address;
    struct sctp_initmsg initmsg;

    fd_set socket_set, ready_socket_set;
    int client_sockets[30], max_clients = 30, activity, max_socket, socket_descriptor;

    // Create threads to handle connections
    for (int i = 0; i < CLIENT_THREAD_POOL_SIZE; i++) {
        pthread_create(
            &clients_thread_pool[i],
            NULL,
            client_connection_thread_function,
            NULL
        );
    }

    check(
        (server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)),
        "Failed to create client socket!"
    );
    
    memset (&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    check(
        (setsockopt(server_socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg))),
        "master socket for clients setsockopt failed"
    );

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(CLIENTS_PORT);

    check(
        bind(
            server_socket,
            (SA*) &server_address,
            sizeof(server_address)
        ),
        "Bind failed!"
    );

    check(
        listen(server_socket, SERVER_BACKLOG),
        "Listen failed!"
    );

    address_size = sizeof(SA_IN);
    for (int i = 0; i < max_clients; i++) {
        client_sockets[i] = -1;
    }
    
    FD_ZERO(&socket_set);
    FD_SET(server_socket, &socket_set);

    while(true) {
        // wait for and accept an incoming connection

        ready_socket_set = socket_set;
        check(select(FD_SETSIZE, &ready_socket_set, NULL, NULL, NULL), "Failed select");

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_socket_set)) {
                if (i == server_socket) {
                    check(
                        (
                            client_socket = accept(
                                server_socket,
                                (SA*)&client_address,
                                (socklen_t*) &address_size
                            )
                        ),
                        "Accept failed!"
                    );


                    // Queue connection so that a client thread can grab it
                    int *pclient = malloc(sizeof(int));
                    *pclient = client_socket;

                    // Prevent race condition
                    pthread_mutex_lock(&client_pool_mutex);
                    enqueue_client_connection(pclient);
                    printf("Queued client connection\n");
                    // pthread_cond_signal(&client_pool_condition_var);
                    sem_post(&sem_client_queue);
                    pthread_mutex_unlock(&client_pool_mutex);
                }
            }
            
        }
    }
}
