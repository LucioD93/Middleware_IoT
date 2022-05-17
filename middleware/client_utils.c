#include "client_utils.h"


void *get_request_params(int request_type) {
//    FILE *f = NULL;
//    char filename[80] = "test.jpg";
//    char buffer[MAXLINE];
//    long file_bytes = 0;
//    char c = '\0';
//    int i = 0;
//
//    f = fopen(filename,"rb");
//    if (f == NULL) {
//        printf("\nError opening file.\n");
//    } else {
//        fseek(f,0,SEEK_END);
//        file_bytes = ftell(f);
//        fseek(f,0,SEEK_SET);
//
//    }
//
//    while (c != EOF){
//        c = fgetc(f);
//        buffer[i] = c;
//        i++;
//    }
//
//    printf("IMAGE SIZE %d\n", i);

    switch (request_type) {
        case 1:
            // image processing
        break;
        case 2:
            // web
        break;
        case 3:
            // word processing
        break;
        case 4:
            // sync processing
        break;
        case 5:
            // image locating
        break;
        case 6:
            // ip locating
        break;
        default:
            return NULL;
        break;
    }
}



void *worker_connection_function(void *args) {
    int request_id = *((int*)args);
    printf("START %d\n", request_id);
    int server_socket, worker_socket, addr_size;
    SA_IN server_addr, worker_addr;

    check(
            (server_socket = socket(AF_INET, SOCK_STREAM, 0)),
            "Failed to create socket!"
    );

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(RESPONSES_PORT);

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

    printf("Waiting worker connection...\n");
    // wait for and accept an incoming connection
    addr_size = sizeof(SA_IN);

    check(
        (worker_socket = accept(
                server_socket,
                (SA*)&worker_addr,
                (socklen_t*) &addr_size
        )),
        "Accept failed!"
    );

    printf("ACCEPTED WORKER CONNECTION\n");

    // TODO: send args

    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t message_size;
    memset(&buffer, 0, BUFFERSIZE);
    bytes_read = 0;
    message_size = 0;
    // read master's message
    printf("READING MASTERS MESSAGE\n");
    while((bytes_read = read(worker_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
        message_size += bytes_read;
        if(message_size > BUFFERSIZE - 1 || buffer[message_size - 1] == 0) break;
    }
    buffer[message_size - 1] = 0; // null terminate

    printf("Got from worker: |%s|\n", buffer);

    check(close(worker_socket), "Socket closing Failed");
}

_Noreturn void client_function(int request_id) {
    printf("Started client %d\n", request_id);
    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAXLINE];

    check(
            (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
            "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(CLIENTS_PORT);

    check(
            (inet_pton(AF_INET, MASTERSERVERADDRESS, &servaddr.sin_addr)),
            "Server address translation failed"
    );

    check(
            (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
            "Connection failed"
    );

    printf("CONNECTED TO MASTER SERVER\n");

    sprintf(sendline,"%d", request_id);
    sendbytes = sizeof(sendline);

    printf("SENDING:[%d][%s]\n", sendbytes, sendline);

    check(
            (write(sockfd, &sendline, sendbytes) != sendbytes),
            "Socket write failed"
    );

    check(close(sockfd), "Socket closing Failed");

    printf("Socket closed\n");

    worker_connection_function(&request_id);

    exit(0);
}
