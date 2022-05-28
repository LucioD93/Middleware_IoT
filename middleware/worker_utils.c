#include "worker_utils.h"

pthread_t master_thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t master_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t master_pool_condition_var = PTHREAD_COND_INITIALIZER;


void get_date_time(char *currentTime) {
    time_t timer;
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(currentTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

void get_random_city(char *city) {
    const char * cities[] = {
        "La Recoleta Museum",
        "La Catedral Museum",
        "Santa Teresa Museum",
        "Santuarios Andinos Museum",
        "Archaeology Museum UNSA",
        "Casa de la Cultura"
    };
    int choice = rand() % 6;
    strcpy(city, cities[choice]);
}

void receive_file_with_socket(char filename[MAXLINE], int socket) {
    FILE * file;
    file = fopen(filename, "w");
    char buffer[MAXLINE];
    size_t bytes_read;
    size_t message_size;
    memset(&buffer, 0, BUFFERSIZE);
    message_size = 0;
    while(true) {
        bzero(buffer, MAXLINE);
        bytes_read = recv(socket, buffer, MAXLINE, 0);
        if (strcmp(buffer, "Finalizado") == 0) break;
        message_size += bytes_read;
        fputs(buffer, file);
    }
    fclose(file);
}


void *handle_master_connection(int *master_socket, int request_id, char *client_ip) {
//    int master_socket = *((int*)p_socket);
//    char buffer[BUFFERSIZE];
//    size_t bytes_read;
//    size_t message_size;
//    memset(&buffer, 0, BUFFERSIZE);
//    bytes_read = 0;
//    message_size = 0;
//    // read master's message
//    printf("READING MASTERS MESSAGE\n");
//    while((bytes_read = read(master_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
//        message_size += bytes_read;
//        if(message_size > BUFFERSIZE - 1 || buffer[message_size - 1] == 0) break;
//    }
//    buffer[message_size - 1] = 0; // null terminate
//
//    char *client_ip = malloc(15);
//    int request_id;
//    sscanf(buffer, "%s | %d", client_ip, &request_id);
//    printf("Received from master %d - %s\n", request_id, client_ip);
//    fflush(stdout);

    // Connect to client socket
    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAXLINE];
    char *metadata_str;

    check(
        (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
        "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(RESPONSES_PORT);

    check(
        (inet_pton(AF_INET, client_ip, &servaddr.sin_addr)),
        "Server address translation failed"
    );

    check(
        (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
        "Connection failed"
    );

    printf("CONNECTED\n");

    if (request_id == 1 || request_id == 3 || request_id == 5) {
        char filename[MAXLINE] = "got.txt";
        receive_file_with_socket(filename, sockfd);
    }
    // TODO: process function params from client

    if (request_id == 2) {
        strcpy(sendline, "http://www.rutas.com.pe");
    } else if (request_id == 4) {
        get_date_time(sendline);
    } else if (request_id == 5 || request_id == 6) {
        get_random_city(sendline);
    }

    sendbytes = sizeof(sendline);
    check(
        (write(sockfd, &sendline, sendbytes) != sendbytes),
        "Socket write failed"
    );

    check(close(sockfd), "Socket closing Failed");
}


_Noreturn void * master_connection_thread_function(void *arg) {
    while (true) {
        pthread_mutex_lock(&master_pool_mutex);
        pthread_cond_wait(&master_pool_condition_var, &master_pool_mutex);
//        int *p_client = dequeue_master_connection();
        node_t result = dequeue_master_connection();
        pthread_mutex_unlock(&master_pool_mutex);
        if (result.socket_descriptor != NULL) {
            // There is a connection
            printf("CLIENT CONNECTED\n");
            handle_master_connection(result.socket_descriptor, result.connection->request_id, result.connection->client_ip);
        }
    }
}

_Noreturn void master_worker_server(void *p_socket) {
    int server_socket, worker_socket, addr_size;
    SA_IN server_addr, worker_addr;

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&master_thread_pool[i], NULL, master_connection_thread_function, NULL);
    }

    int master_socket = *((int*)p_socket);
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t message_size;


    while(true) {

        memset(&buffer, 0, BUFFERSIZE);
        bytes_read = 0;
        message_size = 0;
        // read master's message
        printf("READING MASTERS MESSAGE\n");
        while((bytes_read = read(master_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
            message_size += bytes_read;
            if(message_size > BUFFERSIZE - 1 || buffer[message_size - 1] == 0) break;
        }
        buffer[message_size - 1] = 0; // null terminate

        char *client_ip = malloc(15);
        int request_id;
        sscanf(buffer, "%s | %d", client_ip, &request_id);
        printf("Received from master %d - %s\n", request_id, client_ip);
        fflush(stdout);

        // Queue connection so that a worker thread can grab it
        int *p_client = malloc(sizeof(int));
        *p_client = worker_socket;
        printf("ACCEPTED MASTER CONNECTION\n");

        // Prevent race condition
        pthread_mutex_lock(&master_pool_mutex);
        enqueue_master_connection(p_client, request_id, client_ip);
        pthread_cond_signal(&master_pool_condition_var);
        pthread_mutex_unlock(&master_pool_mutex);
    }
}


_Noreturn void worker_metadata_thread() {
    Metadata worker_metadata = create_worker_metadata();
    printf("WORKER UUID [%s]\n", worker_metadata.uuid);
    printf("CPU %d - RAM %d - GPU %d\n", worker_metadata.resources.cpu, worker_metadata.resources.ram, worker_metadata.resources.gpu);

    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAXLINE];
    char *metadata_str;

    check(
        (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
        "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(WORKERS_PORT);

    check(
        (inet_pton(AF_INET, MASTERSERVERADDRESS, &servaddr.sin_addr)),
        "Server address translation failed"
    );

    check(
        (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
        "Connection failed"
    );

    printf("CONNECTED\n");

    pthread_t worker_connections_thread;
    pthread_create(&worker_connections_thread, NULL, (void *(*)(void *)) master_worker_server, &sockfd);

    while(true) {
        worker_metadata.resources.cpu_usage = get_cpu_usage();
        metadata_str = metadata_to_str(worker_metadata);
        strcpy(sendline, metadata_str);
        sendbytes = sizeof(sendline);
        free(metadata_str);

        printf("SENDING:[%d][%s]\n", sendbytes, sendline);

        check(
            (write(sockfd, &sendline, sendbytes) != sendbytes),
            "Socket write failed"
        );
        sleep(WORKERSYNCTIMER);
    }

}


void worker() {
    worker_metadata_thread();
}
