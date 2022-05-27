#include "worker_utils.h"


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
    while(1) {
        bzero(buffer, MAXLINE);
        bytes_read = recv(socket, buffer, MAXLINE, 0);
        if (strcmp(buffer, "Finalizado") == 0) break;
        message_size += bytes_read;
        fputs(buffer, file);
    }
    fclose(file);
}


void *worker_client_thread(void *p_socket) {
    printf("THREAD STARTED\n");
    int master_socket = *((int*)p_socket);
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t message_size;
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
    pthread_create(&worker_connections_thread, NULL, (void *(*)(void *)) worker_client_thread, &sockfd);


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

    check(close(sockfd), "Socket closing Failed");

    printf("Socket closed\n");
    exit(0);

}


void worker() {
    worker_metadata_thread();
}
