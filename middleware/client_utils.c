#include "client_utils.h"


void *worker_connection_function(int request_type, char filename[MAX_LINE], int server_socket) {
    int worker_socket, address_size;
    SA_IN worker_address;

    check(
        listen(server_socket, 5),
        "Listen failed!"
    );

    // wait for and accept an incoming connection
    address_size = sizeof(SA_IN);

    check(
        (
            worker_socket = accept(
                server_socket,
                (SA*)&worker_address,
                (socklen_t*) &address_size
            )
        ),
        "Accept failed!"
    );

    // This request types need to send an input file
    if (request_type == WORD_PROCESSING_REQUEST) {
        send_text_file_over_socket(filename, worker_socket);
    } else if (
        request_type == IMAGE_PROCESSING_REQUEST ||
        request_type == IMAGE_LOCATION_REQUEST
    ) {
        send_image_file_over_socket(filename, worker_socket);
    }

    if (request_type == WORD_PROCESSING_REQUEST) {
        char *uuid = malloc(sizeof(char)*UUID_STR_LEN);
        generate_uuid(uuid);
        sprintf(filename, "output-%s.txt", uuid);
        receive_text_file_over_socket(filename, worker_socket);
        printf("Response from worker: |%s|\n", filename);
    } else if (request_type == IMAGE_PROCESSING_REQUEST) {
        char *uuid = malloc(sizeof(char)*UUID_STR_LEN);
        generate_uuid(uuid);
        sprintf(filename, "output-%s.jpg", uuid);
        receive_image_file_over_socket(filename, worker_socket);
        printf("Response from worker: |%s|\n", filename);
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

        printf("Response from worker: |%s|\n", buffer);
    }

    check(close(server_socket), "Socket closing Failed");
    check(close(worker_socket), "Socket closing Failed");
}

void client_function(
    int request_type,
    char master_server_address[16]
) {
    if (request_type == 7) {
        request_type = rand() % 6 + 1;
    }

    char filename[MAX_LINE];
    switch(request_type) {
    case WORD_PROCESSING_REQUEST:
        strcpy(filename, "client.txt");
        break;
    case IMAGE_PROCESSING_REQUEST:
    case IMAGE_LOCATION_REQUEST:
        strcpy(filename, "client.jpg");
        break;
    }

    int master_socket, worker_socket, server_socket, bytes_to_send;
    SA_IN client_address, server_address;
    char line_to_send[MAX_LINE];
    struct sctp_initmsg initmsg;

    // Open worker socket first to get the port that will be used
    check(
        (worker_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)),
        "Failed to create client socket!"
    );
    
    memset (&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    check(
        (setsockopt(worker_socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg))),
        "client socket for worker setsockopt failed"
    );

    memset(&server_address, 0, sizeof(server_address));
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

    SA_IN sin;
    int len = sizeof(sin);
    check(
        (getsockname(worker_socket, (SA *)&sin, &len) == -1),
        "Get socket port failed!"
    );
    int worker_port = ntohs(sin.sin_port);

    check(
        (master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)),
        "Failed to create client socket!"
    );
    
    memset (&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = 2000;
    initmsg.sinit_max_instreams = 2000;
    initmsg.sinit_max_attempts = 20;
    check(
        (setsockopt(master_socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg))),
        "client socket for master setsockopts failed"
    );

    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(CLIENTS_PORT);

    check(
        (inet_pton(AF_INET, master_server_address, &client_address.sin_addr)),
        "Server address translation failed"
    );

    check(
        (connect(master_socket, (SA *)&client_address, sizeof(client_address))),
        "Connection to master failed"
    );

    sprintf(line_to_send, "%d-%d", request_type, worker_port);
    bytes_to_send = 8;

    check(
        (write(master_socket, &line_to_send, bytes_to_send) != bytes_to_send),
        "Socket write failed"
    );

    check(close(master_socket), "Closing socket to master Failed");
    worker_connection_function(request_type, filename, worker_socket);

    fflush(stdout);
}
