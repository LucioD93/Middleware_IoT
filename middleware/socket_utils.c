#include "socket_utils.h"


void send_text_file_over_socket(char filename[BUFFER_SIZE], int socket) {
    time_t start, end;
    time(&start);
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
    double time_taken = end - start;
    // printf("Sended text file-->%f\n", time_taken);
}


void send_image_file_over_socket(char filename[BUFFER_SIZE], int socket) {
    time_t start, end;
    time(&start);
    char data[MAX_LINE] = {0};
    FILE *file;
    file = fopen(filename, "rb");
    size_t read_bytes = 1, total_bytes = 0;
    while(read_bytes != 0) {
        read_bytes = fread(data, sizeof(char), MAX_LINE, file);
        total_bytes += read_bytes;
        check(
            send(socket, data, read_bytes, 0) == SOCKET_ERROR,
            "ERROR: File sending failed!\n"
        );
        memset(data, 0, MAX_LINE);
    }
    fclose(file);
    char final = EOF;
    check(
        send(socket, &final, 1, 0) == SOCKET_ERROR,
        "ERROR: File sending failed!\n"
    );
    fflush(stdout);
    time(&end);
    double time_taken = end - start;
    // printf("Sended image file-->%f\n", time_taken);
}


void receive_text_file_over_socket(char filename[MAX_LINE], int socket) {
    time_t start, end;
    time(&start);
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
    double time_taken = end - start;
    // printf("Received text file-->%f\n", time_taken);
}


void receive_image_file_over_socket(char filename[MAX_LINE], int socket) {
    time_t start, end;
    time(&start);
    FILE * file;
    file = fopen(filename, "wb");
    char buffer[MAX_LINE] = {0};
    size_t bytes_read, total_bytes;
    while((bytes_read = recv(socket, &buffer, MAX_LINE, 0)) > 0) {
        total_bytes += bytes_read;
        if (buffer[bytes_read - 1] == EOF && bytes_read != MAX_LINE) {
            int check = 0;
            int i = bytes_read;
            while (check == 0 && i < MAX_LINE) {
                check += buffer[i];
                i++;
            }
            if (check == 0) {
                char temp[MAX_LINE] = {0};
                strncpy(temp, buffer, bytes_read - 1);
                fwrite(buffer, sizeof(char), bytes_read - 1, file);
                break;
            }
        }
        fwrite(buffer, sizeof(char), bytes_read, file);
        memset(buffer, 0, MAX_LINE);
    }
    fclose(file);
    double time_taken = end - start;
    // printf("Received image file-->%f\n", time_taken);
}
