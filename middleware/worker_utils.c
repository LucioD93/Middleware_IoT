#include "worker_utils.h"

pthread_t master_thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t master_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t master_pool_condition_var = PTHREAD_COND_INITIALIZER;

pthread_mutex_t assigned_tasks_mutex = PTHREAD_MUTEX_INITIALIZER;


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


void *handle_master_connection(int request_type, char *client_ip, int client_port) {
    printf("Serving request %d\n", request_type);
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
    servaddr.sin_port = htons(client_port);

    check(
        (inet_pton(AF_INET, client_ip, &servaddr.sin_addr)),
        "Server address translation failed"
    );

    int exp;
    for (int i = 0; i < 3; ++i) {
        exp = connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
        if (exp != SOCKET_ERROR) break;
        sleep(i);
    }
    check(exp,"Connection to client failed");

    if (request_type == WORD_PROCESSING_REQUEST) {
        char filename[MAX_LINE] = "worker.txt";
        strcpy(sendline, filename);
        receive_text_file_over_socket(filename, sockfd);
    } else if (
            request_type == IMAGE_PROCESSING_REQUEST ||
            request_type == IMAGE_LOCATION_REQUEST
    ) {
        char filename[MAX_LINE] = "worker.jpg";
        strcpy(sendline, "worker.jpg");
        receive_image_file_over_socket(filename, sockfd);
    }
    // TODO: process function params from client

    if (request_type == IMAGE_PROCESSING_REQUEST) {
        char filename[MAX_LINE] = "worker.jpg";
        send_image_file_over_socket(filename, sockfd);
    } else if (request_type == WORD_PROCESSING_REQUEST) {
        char filename[MAX_LINE] = "worker.txt";
        send_text_file_over_socket(filename, sockfd);
    } else {
        if (request_type == WEB_REQUEST) {
            strcpy(sendline, "http://www.rutas.com.pe");
        } else if (request_type == SYNCHRONIZATION_REQUEST) {
            get_date_time(sendline);
        } else if (request_type == IMAGE_LOCATION_REQUEST || request_type == IP_LOCATION_REQUEST) {
            get_random_city(sendline);
        }

        sendbytes = sizeof(sendline);
        check(
            (write(sockfd, &sendline, sendbytes) != sendbytes),
            "Socket write failed"
        );
    }

    check(close(sockfd), "Socket closing Failed");
}


_Noreturn void * master_connection_thread_function(void *arg) {
    int *tasks_tracker = arg;
    while (true) {
        pthread_mutex_lock(&master_pool_mutex);
        pthread_cond_wait(&master_pool_condition_var, &master_pool_mutex);
        node_t result = dequeue_master_connection();
        pthread_mutex_unlock(&master_pool_mutex);
        if (result.socket_descriptor != NULL) {
            // There is a connection
            pthread_mutex_lock(&assigned_tasks_mutex);
            (*tasks_tracker)++;
            pthread_mutex_unlock(&assigned_tasks_mutex);
            handle_master_connection(result.connection->request_type, result.connection->client_ip, result.connection->client_port);
            pthread_mutex_lock(&assigned_tasks_mutex);
            (*tasks_tracker)--;
            pthread_mutex_unlock(&assigned_tasks_mutex);
        }
    }
}

_Noreturn void master_worker_server(void *args) {
    worker_args actual_args = *((worker_args*)args);
    int worker_socket;

    // Create threads to handle connections
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(
            &master_thread_pool[i],
            NULL,
            master_connection_thread_function,
            actual_args.tasks_tracker
        );
    }

    int master_socket = *actual_args.socket;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t message_size;

    while(true) {
        memset(&buffer, 0, BUFFER_SIZE);
        message_size = 0;
        // read master's message
        while((bytes_read = read(master_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
            message_size += bytes_read;
            if(message_size > BUFFER_SIZE - 1 || buffer[message_size - 1] == 0) break;
        }
        buffer[message_size - 1] = 0; // null terminate

        char *client_ip = malloc(15);
        int request_type, client_port;
        sscanf(buffer, "%s | %d-%d", client_ip, &request_type, &client_port);
        fflush(stdout);

        // Queue connection so that a worker thread can grab it
        int *p_client = malloc(sizeof(int));
        *p_client = worker_socket;

        // Prevent race condition
        pthread_mutex_lock(&master_pool_mutex);
        enqueue_master_connection(p_client, request_type, client_ip, client_port);
        pthread_cond_signal(&master_pool_condition_var);
        pthread_mutex_unlock(&master_pool_mutex);

    }
}


_Noreturn void worker_metadata_thread(char master_server_address[16]) {
    Metadata worker_metadata = create_worker_metadata();
    printf(
        "WORKER UUID [%s]\nCPU %d - RAM %d - GPU %d - Max Tasks %d\n",
        worker_metadata.uuid,
        worker_metadata.resources.cpu,
        worker_metadata.resources.ram,
        worker_metadata.resources.gpu,
        worker_metadata.resources.max_tasks
    );

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

    printf("Connected to master!\n");

    pthread_t worker_connections_thread;
    worker_args *args = malloc(sizeof(worker_args));
    args->socket = malloc(sizeof(int));
    *args->socket = sockfd;
    args->tasks_tracker = &worker_metadata.resources.assigned_tasks;
    pthread_create(
        &worker_connections_thread,
        NULL,
        (void *(*)(void *)) master_worker_server,
        args
    );

    while(true) {
        worker_metadata.resources.cpu_usage = get_cpu_usage();
        metadata_str = metadata_to_str(worker_metadata);
        strcpy(sendline, metadata_str);

        long long milliseconds = milliseconds_since_epoch();
        char milliseconds_str[256];
        sprintf(milliseconds_str, "%lld\n", milliseconds);
        strcat(sendline, milliseconds_str);

        sendbytes = sizeof(sendline);
        free(metadata_str);

        printf("SENDING: %s\n", sendline);
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
