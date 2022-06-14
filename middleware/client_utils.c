#include "client_utils.h"


void *worker_connection_function(int request_id, char filename[MAX_LINE], int server_socket) {
    int worker_socket, address_size;
    SA_IN worker_address;

    check(
        listen(server_socket, SERVER_BACKLOG),
        "Listen failed!"
    );

    // wait for and accept an incoming connection
    address_size = sizeof(SA_IN);

    check(
    (worker_socket = accept(
        server_socket,
        (SA*)&worker_address,
        (socklen_t*) &address_size
    )),
    "Accept failed!"
    );

    // This request types need to send an input file
    if (request_id == WORD_PROCESSING_REQUEST) {
        send_text_file_over_socket(filename, worker_socket);
    } else if (
        request_id == IMAGE_PROCESSING_REQUEST ||
        request_id == IMAGE_LOCATION_REQUEST
    ) {
        send_image_file_over_socket(filename, worker_socket);
    }

    if (request_id == WORD_PROCESSING_REQUEST) {
        strcpy(filename, "client_output.txt");
        receive_text_file_over_socket(filename, worker_socket);
        printf("Received text file from worker. check %s\n", filename);
    } else if (request_id == IMAGE_PROCESSING_REQUEST) {
        strcpy(filename, "client_output.jpg");
        receive_image_file_over_socket(filename, worker_socket);
        printf("Received text file from image. check %s\n", filename);
    } else {
        char buffer[BUFFER_SIZE];
        size_t bytes_read;
        size_t message_size;
        memset(&buffer, 0, BUFFER_SIZE);
        message_size = 0;

        // read master's message
        while(
            (bytes_read = read(worker_socket, buffer + message_size, sizeof(buffer) - message_size)) > 0
        ) {
            message_size += bytes_read;
            if(message_size > BUFFER_SIZE - 1 || buffer[message_size - 1] == 0) break;
        }
        buffer[message_size - 1] = 0; // null terminate

        printf("Got from worker: |%s|\n", buffer);
    }

    check(close(server_socket), "Socket closing Failed");
    check(close(worker_socket), "Socket closing Failed");
}

_Noreturn void client_function(
    int request_id,
    char filename[MAX_LINE],
    char master_server_address[16]
) {
    int master_socket, worker_socket, server_socket, bytes_to_send;
    SA_IN client_address, server_address;
    char line_to_send[MAX_LINE];

    // Open worker socket first to get the port that will be used
    check(
        (worker_socket = socket(AF_INET, SOCK_STREAM, 0)),
        "Client socket for worker creation failed"
    );
    int opt = 1;
    check(
    (
            setsockopt(
                worker_socket,
                SOL_SOCKET,
                SO_REUSEADDR | SO_REUSEPORT,
                &opt, sizeof(opt)
            )
        ),
        "Set socket option failed"
    );

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(0);

    check(
        bind(
            worker_socket,
            (SA*) &server_address,
            sizeof(server_address)
        ),
        "Bind failed!"
    );

    struct sockaddr_in sin;
    int len = sizeof(sin);
    check(
        (getsockname(worker_socket, (struct sockaddr *)&sin, &len) == -1),
        "Get socket port failed!"
    );
    int worker_port = ntohs(sin.sin_port);
    printf("port number %d\n", worker_port);

    check(
        (master_socket = socket(AF_INET, SOCK_STREAM, 0)),
        "Client socket creation failed"
    );

    bzero(&client_address, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(CLIENTS_PORT);

    check(
        (inet_pton(AF_INET, master_server_address, &client_address.sin_addr)),
        "Server address translation failed"
    );

    check(
        (connect(master_socket, (SA *) &client_address, sizeof(client_address))),
        "Connection failed"
    );

    sprintf(line_to_send, "%d-%d", request_id, worker_port);
    bytes_to_send = sizeof(line_to_send);

    check(
        (write(master_socket, &line_to_send, bytes_to_send) != bytes_to_send),
        "Socket write failed"
    );

    check(close(master_socket), "Closing socket to Master Failed");
    worker_connection_function(request_id, filename, worker_socket);

    fflush(stdout);
    exit(0);
}
