#include "worker_utils.h"

pthread_t* master_thread_pool;
pthread_mutex_t master_pool_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t assigned_tasks_mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t sem_consumer;
sem_t sem_producer;

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
    printf("Worker started serving request %d %s:%d\n", request_type, client_ip, client_port);
    // Connect to client socket
    int client_socket, send_bytes;
    SA_IN server_address;
    char send_line[MAX_LINE];
    struct sctp_initmsg initmsg;

    check(
        (client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)),
        "Failed to create client socket!"
    );
    
    memset (&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = 2048;
    initmsg.sinit_max_instreams = 2048;
    initmsg.sinit_max_attempts = 20;
    check(
        (setsockopt(client_socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg))),
        "worker socket to master setsockopt failed"
    );

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(client_port);

    check(
        (inet_pton(AF_INET, client_ip, &server_address.sin_addr)),
        "Server address translation failed"
    );

    int exp;

    for (int i = 1; i <= 3; ++i) {
        exp = connect(
            client_socket, (SA *)&server_address, sizeof(server_address)
        );
        if (exp != SOCKET_ERROR) break;
        sleep_for_milliseconds(250 * i);
    }
    check(exp, "Connection to client failed");

    char filename[MAX_LINE];

    if (request_type == WORD_PROCESSING_REQUEST) {
        char *uuid = malloc(sizeof(char)*UUID_STR_LEN);
        generate_uuid(uuid);
        sprintf(filename, "temp-%s.txt", uuid);
        receive_text_file_over_socket(filename, client_socket);
    } else if (
        request_type == IMAGE_PROCESSING_REQUEST ||
        request_type == IMAGE_LOCATION_REQUEST
    ) {
        char *uuid = malloc(sizeof(char)*UUID_STR_LEN);
        generate_uuid(uuid);
        sprintf(filename, "temp-%s.jpg", uuid);
        receive_image_file_over_socket(filename, client_socket);
    }

    if (request_type == IMAGE_PROCESSING_REQUEST) {
        send_image_file_over_socket(filename, client_socket);
    } else if (request_type == WORD_PROCESSING_REQUEST) {
        send_text_file_over_socket(filename, client_socket);
    } else {
        if (request_type == WEB_REQUEST) {
            strcpy(send_line, "http://www.rutas.com.pe");
        } else if (request_type == SYNCHRONIZATION_REQUEST) {
            get_date_time(send_line);
        } else if (request_type == IMAGE_LOCATION_REQUEST || request_type == IP_LOCATION_REQUEST) {
            get_random_city(send_line);
        }

        send_bytes = strlen(send_line) + 1;
        check(
            (write(client_socket, &send_line, send_bytes) != send_bytes),
            "Socket write failed"
        );
    }

    check(close(client_socket), "Socket closing Failed");
}


_Noreturn void * master_connection_thread_function(void *arg) {
    int *tasks_tracker = arg;
    while (true) {
        // Semaphores!
        sem_wait(&sem_producer);
        pthread_mutex_lock(&master_pool_mutex);
        node_t *result = dequeue_master_connection();
        pthread_mutex_unlock(&master_pool_mutex);
        sem_post(&sem_consumer);

        if (result != NULL) {
            // There is a connection
            pthread_mutex_lock(&assigned_tasks_mutex);
            *tasks_tracker = *tasks_tracker + 1;
            pthread_mutex_unlock(&assigned_tasks_mutex);
            handle_master_connection(
                result->connection->request_type,
                result->connection->client_ip,
                result->connection->client_port
            );
            pthread_mutex_lock(&assigned_tasks_mutex);
            *tasks_tracker = *tasks_tracker - 1;
            pthread_mutex_unlock(&assigned_tasks_mutex);
            free(result->connection->client_ip);
            free(result->connection);
            free(result);
        }
    }
}


_Noreturn void master_worker_server(void *args) {
    worker_args actual_args = *((worker_args*)args);
    int worker_socket;

    int master_socket = *actual_args.socket;
    int thread_pool_size = actual_args.thread_pool_size;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t message_size;

    // Create threads to handle connections
    master_thread_pool = (pthread_t*)malloc(sizeof(pthread_t)*thread_pool_size);
    for (int i = 0; i < thread_pool_size; i++) {
        pthread_create(
            &master_thread_pool[i],
            NULL,
            master_connection_thread_function,
            actual_args.tasks_tracker
        );
    }

    while(true) {
        memset(&buffer, 0, BUFFER_SIZE);
        message_size = 0;
        // read master's message
        while((bytes_read = read(master_socket, buffer + message_size, 26 - message_size)) > 0) {
            message_size += bytes_read;
            if(message_size > 26 || buffer[message_size - 1] == 0) break;
        }
        buffer[message_size - 1] = 0; // null terminate
        printf("Worker got %s (%ld)\n", buffer, message_size);

        char *client_ip = malloc(15);
        int request_type, client_port;
        sscanf(buffer, "%s | %d-%d", client_ip, &request_type, &client_port);\
        fflush(stdout);

        // Queue connection so that a worker thread can grab it
        int *p_client = malloc(sizeof(int));
        *p_client = worker_socket;

        // Prevent race condition with semaphores!
        sem_wait(&sem_consumer);
        pthread_mutex_lock(&master_pool_mutex);
        enqueue_master_connection(p_client, request_type, client_ip, client_port);
        pthread_mutex_unlock(&master_pool_mutex);
        sem_post(&sem_producer);
    }
}


_Noreturn void worker_metadata_thread(char master_server_address[16], int gpu) {
    Metadata worker_metadata = create_worker_metadata(gpu);
    printf(
        "WORKER UUID [%s]\nCPU %d - RAM %d - GPU %d\n Max Tasks %d\n",
        worker_metadata.uuid,
        worker_metadata.resources.cpu,
        worker_metadata.resources.ram,
        worker_metadata.resources.gpu,
        worker_metadata.resources.max_tasks
    );

    int master_socket, send_bytes;
    SA_IN server_address;
    char send_line[MAX_LINE];
    char *metadata_str;
    struct sctp_initmsg initmsg;

    check(
        (master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)),
        "Failed to create client socket!"
    );
    
    memset (&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    check(
        (setsockopt(master_socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg))),
        "worker socket to client setsockopt failed"
    );

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(WORKERS_PORT);

    check(
        (inet_pton(AF_INET, master_server_address, &server_address.sin_addr)),
        "Server address translation failed"
    );

    check(
        (connect(master_socket, (SA *)&server_address, sizeof(server_address))),
        "Connection to Master failed"
    );

    pthread_t worker_connections_thread;
    worker_args *args = malloc(sizeof(worker_args));
    args->socket = malloc(sizeof(int));
    *args->socket = master_socket;
    args->tasks_tracker = &worker_metadata.resources.assigned_tasks;
    args->thread_pool_size = worker_metadata.resources.max_tasks;
    pthread_create(
        &worker_connections_thread,
        NULL,
        (void *(*)(void *)) master_worker_server,
        args
    );

    while(true) {
        worker_metadata.resources.cpu_usage = get_cpu_usage();
        metadata_str = metadata_to_str(worker_metadata);
        strcpy(send_line, metadata_str);

        long long milliseconds = milliseconds_since_epoch();
        char milliseconds_str[256];
        sprintf(milliseconds_str, "%lld\n", milliseconds);
        strcat(send_line, milliseconds_str);

        send_bytes = sizeof(send_line);
        free(metadata_str);

        check(
            (write(master_socket, &send_line, send_bytes) != send_bytes),
            "Socket write failed"
        );
        sleep(WORKER_SYNC_TIMER);
    }
}


void worker(char master_server_address[16], int gpu) {
    sem_init(&sem_producer, 0, 0);
    sem_init(&sem_consumer, 0, 1);
    worker_metadata_thread(master_server_address, gpu);
}
