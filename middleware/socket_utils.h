#ifndef MIDDLEWARE_SOCKET_UTILS_H
#define MIDDLEWARE_SOCKET_UTILS_H

#include <stdio.h>
#include "threading_utils.h"
#include "constants.h"

void send_text_file_over_socket(char filename[BUFFER_SIZE], int socket);
void send_image_file_over_socket(char filename[BUFFER_SIZE], int socket);

void receive_text_file_over_socket(char filename[MAX_LINE], int socket);
void receive_image_file_over_socket(char filename[MAX_LINE], int socket);

#endif //MIDDLEWARE_SOCKET_UTILS_H
