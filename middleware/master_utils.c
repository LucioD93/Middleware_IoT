#include "master_utils.h"

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;


_Noreturn void * handle_connection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    printf("Got a connection from %d\n", client_socket);
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t message_size;

    while(true) {
        memset(&buffer, 0, BUFFERSIZE);
        bytes_read = 0;
        message_size = 0;
        // read the client's message
        while((bytes_read = read(client_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
            message_size += bytes_read;
            if(message_size > BUFFERSIZE - 1 || buffer[message_size - 1] == 0) break;
        }
        check(bytes_read, "Receiving failed!");
        buffer[message_size - 1] = 0; // null terminate

        printf("Request (%lu)(%lu): [%s]\n", message_size, strlen(buffer), buffer);

        Metadata worker_metadata = str_to_metadata(buffer);
        printf("METADATA RECEIVED\n");
        print_metadata(worker_metadata);
        printf("===========\n");

        pthread_mutex_lock(&mutex);
        add_to_list(worker_metadata);
        pthread_mutex_unlock(&mutex);

        show_list();

        fflush(stdout);
    }
}


_Noreturn void * thread_function(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condition_var, &mutex);
        int *pclient = dequeue();
        pthread_mutex_unlock(&mutex);
        if (pclient != NULL) {
            // There is a connection
            handle_connection(pclient);
        }
    }
}


_Noreturn void master_server() {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    check(
            (server_socket = socket(AF_INET, SOCK_STREAM, 0)),
            "Failed to create socket!"
    );

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVERPORT);

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
        printf("Waiting connections...\n");
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
        printf("ACCEPTED\n");

        // Prevent race condition
        pthread_mutex_lock(&mutex);
        enqueue(pclient);
        pthread_cond_signal(&condition_var);
        pthread_mutex_unlock(&mutex);
    }
}
