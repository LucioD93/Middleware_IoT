#include "threading_utils.h"

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;


int check(int exp, const char *msg) {
    if (exp == SOCKETERROR) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void * handle_connection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    printf("Got a connection from %d\n", client_socket);

    char buffer[BUFFERSIZE];
    size_t bytes_read;
    int message_size = 0;

    // read the client's message
    while((bytes_read = read(client_socket, buffer + message_size, sizeof(buffer) - message_size - 1)) > 0) {
        message_size += bytes_read;
        if(message_size > BUFFERSIZE - 1 || buffer[message_size - 1] == '\n') break;
    }
    check(bytes_read, "Receiving failed!");
    buffer[message_size - 1] = 0; // null terminate

    printf("Request: [%s]\n", buffer);

    // Send response
    write(client_socket, "Thanks for coming!\n", 19);

    fflush(stdout);
    return NULL;
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
        show_queue();
        pthread_cond_signal(&condition_var);
        pthread_mutex_unlock(&mutex);
    }
}


void worker(Metadata worker_metadata) {
    int sockfd, n, sendbytes;
    SA_IN servaddr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    check(
    (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
    "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVERPORT);

    check(
            (inet_pton(AF_INET, SERVERADDRESS, &servaddr.sin_addr)),
            "Server address translation failed"
    );

    check(
            (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
            "Connection failed"
    );

    sprintf(sendline, "HELLO FROM WORKER %s\n", worker_metadata.uuid);
    sendbytes = strlen(sendline);

    check(
            (write(sockfd, &sendline, sendbytes) != sendbytes),
            "Socket write failed"
    );

    memset(recvline, 0, MAXLINE);

    while (( n = read(sockfd, recvline, MAXLINE - 1)) > 0) {
        printf("%s", recvline);
    }

    check((n > 0), "socket read failed");
    printf("WORKER FINISHED");
}