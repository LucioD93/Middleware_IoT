#include "client_utils.h"


void *worker_connection_function(int request_id, char filename[MAX_LINE]) {
    int server_socket, worker_socket, addr_size;
    SA_IN server_addr, worker_addr;

    check(
        (server_socket = socket(AF_INET, SOCK_STREAM, 0)),
        "Failed to create socket!"
    );
    int opt = 1;
    check(
            (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))),
            "setsockopt(SO_REUSEADDR) failed"
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
        listen(server_socket, SERVER_BACKLOG),
        "Listen failed!"
    );

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

    if (request_id == WORD_PROCESSING_REQUEST) {
        send_text_file_over_socket(filename, worker_socket);
    } else if (
        request_id == IMAGE_PROCESSING_REQUEST ||
        request_id == IMAGE_LOCATION_REQUEST
    ) {
        send_image_file_over_socket(filename, worker_socket);
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t message_size;
    memset(&buffer, 0, BUFFER_SIZE);
    message_size = 0;
    // read master's message
    while((bytes_read = read(worker_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0) {
        message_size += bytes_read;
        if(message_size > BUFFER_SIZE - 1 || buffer[message_size - 1] == 0) break;
    }
    buffer[message_size - 1] = 0; // null terminate

    printf("Got from worker: |%s|\n", buffer);

    check(close(server_socket), "Socket closing Failed");
    check(close(worker_socket), "Socket closing Failed");
}

_Noreturn void client_function(
    int request_id,
    char filename[MAX_LINE],
    char master_server_address[16]
) {
    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAX_LINE];

    check(
        (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
        "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(CLIENTS_PORT);

    check(
        (inet_pton(AF_INET, master_server_address, &servaddr.sin_addr)),
        "Server address translation failed"
    );

    check(
        (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
        "Connection failed"
    );

    sprintf(sendline,"%d", request_id);
    sendbytes = sizeof(sendline);

    check(
        (write(sockfd, &sendline, sendbytes) != sendbytes),
        "Socket write failed"
    );

    check(close(sockfd), "Closing socket to Master Failed");
    worker_connection_function(request_id, filename);

    fflush(stdout);
    exit(0);
}
