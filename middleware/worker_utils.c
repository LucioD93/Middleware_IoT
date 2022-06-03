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

void receive_file_with_socket(char filename[MAX_LINE], int socket) {
    FILE * file;
    file = fopen(filename, "w");
    char buffer[MAX_LINE];
    memset(&buffer, 0, BUFFER_SIZE);
    while(true) {
        bzero(buffer, MAX_LINE);
        recv(socket, buffer, MAX_LINE, 0);
        if (strcmp(buffer, "Finalizado") == 0) break;
        fputs(buffer, file);
    }
    fclose(file);
}

void receive_image_file_over_socket(char filename[MAX_LINE], int socket) {
    FILE * file;
    file = fopen(filename, "wb");
    char buffer[MAX_LINE];
    size_t bytes_read;
    while((bytes_read = recv(socket, buffer, MAX_LINE, 0)) > 0) {
        if (strcmp(buffer, "Finalizado") == 0) break;
        fwrite(buffer, sizeof(char), MAX_LINE, file);
        bzero(buffer, MAX_LINE);
    }
    fclose(file);
}


void *handle_master_connection(int request_id, char *client_ip) {
    // Connect to client socket
    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAX_LINE];

    check(
        (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
        "Client socket creation failed"
    );
    int opt = 1;
    check(
        (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))),
        "setsockopt(SO_REUSEADDR) failed"
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
        "Connection to client failed"
    );

    if (request_id == WORD_PROCESSING_REQUEST) {
        char filename[MAX_LINE] = "worker.txt";
        strcpy(sendline, filename);
        receive_file_with_socket(filename, sockfd);
    } else if (
        request_id == IMAGE_PROCESSING_REQUEST ||
        request_id == IMAGE_LOCATION_REQUEST
    ) {
        char filename[MAX_LINE] = "worker.jpg";
        strcpy(sendline, "worker.jpg");
        receive_image_file_over_socket(filename, sockfd);
        printf("LLEGO\n");
    }
    // TODO: process function params from client

    if (request_id == WEB_REQUEST) {
        strcpy(sendline, "http://www.rutas.com.pe");
    } else if (request_id == SYNCHRONIZATION_REQUEST) {
        get_date_time(sendline);
    } else if (request_id == IMAGE_LOCATION_REQUEST || request_id == IP_LOCATION_REQUEST) {
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
        node_t result = dequeue_master_connection();
        pthread_mutex_unlock(&master_pool_mutex);
        if (result.socket_descriptor != NULL) {
            // There is a connection
            handle_master_connection(result.connection->request_id, result.connection->client_ip);
        }
    }
}

_Noreturn void master_worker_server(void *p_socket) {
    int worker_socket;

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&master_thread_pool[i], NULL, master_connection_thread_function, NULL);
    }

    int master_socket = *((int*)p_socket);
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t message_size;


    while(true) {
        memset(&buffer, 0, BUFFER_SIZE);
        bytes_read;
        message_size = 0;
        // read master's message
        while((bytes_read = read(master_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
            message_size += bytes_read;
            if(message_size > BUFFER_SIZE - 1 || buffer[message_size - 1] == 0) break;
        }
        buffer[message_size - 1] = 0; // null terminate

        char *client_ip = malloc(15);
        int request_id;
        sscanf(buffer, "%s | %d", client_ip, &request_id);
        fflush(stdout);

        // Queue connection so that a worker thread can grab it
        int *p_client = malloc(sizeof(int));
        *p_client = worker_socket;

        // Prevent race condition
        pthread_mutex_lock(&master_pool_mutex);
        enqueue_master_connection(p_client, request_id, client_ip);
        pthread_cond_signal(&master_pool_condition_var);
        pthread_mutex_unlock(&master_pool_mutex);
    }
}


_Noreturn void worker_metadata_thread(char master_server_address[16]) {
    Metadata worker_metadata = create_worker_metadata();
    printf("WORKER UUID [%s]\n", worker_metadata.uuid);
    printf("CPU %d - RAM %d - GPU %d\n", worker_metadata.resources.cpu, worker_metadata.resources.ram, worker_metadata.resources.gpu);

    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAX_LINE];
    char *metadata_str;

    check(
        (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
        "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(WORKERS_PORT);

    check(
        (inet_pton(AF_INET, master_server_address, &servaddr.sin_addr)),
        "Server address translation failed"
    );

    check(
        (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
        "Connection to Master failed"
    );

    pthread_t worker_connections_thread;
    pthread_create(
        &worker_connections_thread,
        NULL,
        (void *(*)(void *)) master_worker_server,
        &sockfd
    );

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
        sleep(WORKER_SYNC_TIMER);
    }

}


void worker(char master_server_address[16]) {
    worker_metadata_thread(master_server_address);
}
