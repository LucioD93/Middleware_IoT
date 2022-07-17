#include "socket_utils.h"


void send_text_file_over_socket(char filename[BUFFER_SIZE], int socket) {
    char data[MAX_LINE] = {0};
    FILE *file;
    file = fopen(filename, "r");
    size_t total_bytes;
    while(fgets(data, MAX_LINE, file) != NULL) {
        check(
            send(socket, data, strlen(data), 0) == SOCKET_ERROR,
            "ERROR: File sending failed!\n"
        );
        total_bytes += strlen(data);
        memset(data, 0, MAX_LINE);
    }
    fclose(file);
    char final = EOF;
    check(
        send(socket, &final, 1, 0) == SOCKET_ERROR,
        "ERROR: File sending failed!\n"
    );
    fflush(stdout);
}


void send_image_file_over_socket(char filename[BUFFER_SIZE], int socket) {
    char data[FILE_BUFFER_SIZE] = {0};
    FILE *file;
    file = fopen(filename, "rb");
    size_t read_bytes = 1;
    while(read_bytes != 0) {
        read_bytes = fread(data, sizeof(char), FILE_BUFFER_SIZE, file);
        check(
            send(socket, data, read_bytes, 0) == SOCKET_ERROR,
            "ERROR: File sending failed!\n"
        );
        memset(data, 0, FILE_BUFFER_SIZE);
    }
    fclose(file);
    char final = EOF;
    check(
        send(socket, &final, 1, 0) == SOCKET_ERROR,
        "ERROR: File sending failed!\n"
    );
    fflush(stdout);
}


void receive_text_file_over_socket(char filename[MAX_LINE], int socket) {
    FILE * file;
    file = fopen(filename, "w");
    char buffer[MAX_LINE] = {0};
    size_t bytes_read;
    while((bytes_read = recv(socket, &buffer, MAX_LINE, 0)) > 0) {
        if (buffer[bytes_read - 1] == EOF) {
            char temp[MAX_LINE] = {0};
            strncpy(temp, buffer, bytes_read - 1);
            fputs(temp, file);
            break;
        }
        fputs(buffer, file);
        memset(buffer, 0, MAX_LINE);
    }
    fclose(file);
}


void receive_image_file_over_socket(char filename[MAX_LINE], int socket) {
    FILE * file;
    file = fopen(filename, "wb");
    char buffer[FILE_BUFFER_SIZE] = {0};
    size_t bytes_read;
    while((bytes_read = recv(socket, &buffer, FILE_BUFFER_SIZE, 0)) > 0) {
        if (buffer[bytes_read - 1] == EOF) {
            fwrite(buffer, sizeof(char), bytes_read - 1, file);
            break;
        }
        fwrite(buffer, sizeof(char), bytes_read, file);
        memset(buffer, 0, FILE_BUFFER_SIZE);
    }
    fclose(file);
}
