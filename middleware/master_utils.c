#include "master_utils.h"

pthread_t workers_thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t worker_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t worker_pool_condition_var = PTHREAD_COND_INITIALIZER;

pthread_t clients_thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t client_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t client_pool_condition_var = PTHREAD_COND_INITIALIZER;

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
        if(strlen(buffer) == 0) {
            // Bad message. Close connection
            remove_from_list(uuid);
            check((close(worker_socket)), "Worker socket closing failed!");
        }
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


void * handle_client_connection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t message_size;
    memset(&buffer, 0, BUFFER_SIZE);
    bytes_read = 0;
    message_size = 0;
    // read the client's message
    while((bytes_read = read(client_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
        message_size += bytes_read;
        if(message_size > BUFFER_SIZE - 1 || buffer[message_size - 1] == 0) break;
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
    }
    printf("Assigned %s to request %d\n", selected_worker->worker_metadata->uuid, request_type);

    SA_IN client_address;
    int len;
    len = sizeof(client_address);
    check(
        getpeername(client_socket, (SA *)&client_address, &len),
        "Failed getpeername"
    );

    char client_connection[MAX_LINE];
    strcpy(client_connection, inet_ntoa(client_address.sin_addr));
    strcat(client_connection, " | ");
    strcat(client_connection, buffer);
    int sendbytes = sizeof (client_connection);

    // Send client ip to worker socket
    check(
        (write(selected_worker->worker_socket, &client_connection, sendbytes) != sendbytes),
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
        pthread_mutex_lock(&client_pool_mutex);
        pthread_cond_wait(&client_pool_condition_var, &client_pool_mutex);
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

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(
            &workers_thread_pool[i],
            NULL,
            worker_connection_thread_function,
            NULL
        );
    }

    check(
        (server_socket = socket(AF_INET, SOCK_STREAM, 0)),
        "Failed to create socket!"
    );
    int opt = 1;
    check(
        (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))),
        "setsockopt(SO_REUSEADDR) failed"
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
    int server_socket, client_socket, address_size;
    SA_IN server_address, client_address;

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(
            &clients_thread_pool[i],
            NULL,
            client_connection_thread_function,
            NULL
        );
    }

    check(
        (server_socket = socket(AF_INET, SOCK_STREAM, 0)),
        "Failed to create socket!"
    );
    int opt = 1;
    check(
        (
                setsockopt(
                    server_socket,
                    SOL_SOCKET,
                    SO_REUSEADDR | SO_REUSEPORT,
                    &opt,
                    sizeof(opt)
                )
            ),
        "setsockopt(SO_REUSEADDR) failed"
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

    while(true) {
        // wait for and accept an incoming connection
        address_size = sizeof(SA_IN);

        check(
            (client_socket = accept(
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
        pthread_cond_signal(&client_pool_condition_var);
        pthread_mutex_unlock(&client_pool_mutex);
    }
}
