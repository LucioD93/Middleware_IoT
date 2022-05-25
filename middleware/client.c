#include "client_utils.h"

void print_usage() {
    printf("-r [1-6]   Tipo de peticion\n");
    printf("      1      Procesamiento de imagen\n");
    printf("      2      Consulta web\n");
    printf("      3      Procesamiento de palabras\n");
    printf("      4      Sincronizacion\n");
    printf("      5      Localizacion por imagen\n");
    printf("      6      Localizacion por direccion IP\n");
}

bool isValidIpAddress(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

int main(int argc, char *argv[]) {
    int option, request_type;
    char filename[MAXLINE] = "test.txt";
    char master_server_address[16] = "127.0.0.1";
    while((option = getopt(argc, argv, "r:a:")) != -1) {
        printf("OPTION -%d\n", option);
        switch(option){
            case 'r': // Request type
                printf("Given r Option: %s\n", optarg);
                request_type = atoi(optarg);
                break;
            case 'a': // Master server address
                strcpy(master_server_address, optarg);
                break;
            case 'h':
                print_usage();
                exit(0);
            default: //used for some unknown options
                printf("unknown option: %c\n", optopt);
                print_usage();
                break;
        }
    }

    if (request_type < 1 || request_type > 6) {
        printf("Error! Tipo de peticion debe estar entre 1 y 6\n");
        exit(1);
    }
    if (!isValidIpAddress(master_server_address)) {
        printf("Invalid IP address for master server\n");
        exit(1);
    }
    printf("MAIN %s\n", filename);
    client_function(request_type, filename);
}
