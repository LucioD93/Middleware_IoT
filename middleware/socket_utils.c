#include "socket_utils.h"

void send_text_file_over_socket(char filename[BUFFER_SIZE], int socket) {
    char data[MAX_LINE] = {0};
    FILE *file;
    file = fopen(filename, "r");
    while(fgets(data, MAX_LINE, file) != NULL) {
        check(
                send(socket, data, sizeof(data), 0) == SOCKET_ERROR,
                "ERROR: File sending failed!\n"
        );
        bzero(data, MAX_LINE);
    }
    fclose(file);
    char final[] = "Finalizado";
    check(
            send(socket, final, 10, 0) == SOCKET_ERROR,
            "ERROR: File sending failed!\n"
    );
    fflush(stdout);
}


void send_image_file_over_socket(char filename[BUFFER_SIZE], int socket) {
    char data[MAX_LINE] = {0};
    FILE *file;
    file = fopen(filename, "rb");
    while(!feof(file)) {
        fread(data, sizeof(char), MAX_LINE, file);
        check(
                send(socket, data, sizeof(data), 0) == SOCKET_ERROR,
                "ERROR: File sending failed!\n"
        );
    }
    fclose(file);
    char final[] = "Finalizado";
    check(
            send(socket, final, 10, 0) == SOCKET_ERROR,
            "ERROR: File sending failed!\n"
    );
    fflush(stdout);
}

void receive_text_file_over_socket(char filename[MAX_LINE], int socket) {
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

