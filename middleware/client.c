#include "client_utils.h"

void print_usage() {
    printf("-r [1-6]   Tipo de peticion\n");
    printf("      1      Procesamiento de imagen\n");
    printf("      2      Consulta web\n");
    printf("      3      Procesamiento de palabras\n");
    printf("      4      Sincronizacion\n");
    printf("      5      Localizacion por imagen\n");
    printf("      6      Localizacion por direccion IP\n");
    printf("-a [ip]    Direccion IP de servidor maestro\n");
}


int main(int argc, char *argv[]) {
    int option, request_type;
    char filename[MAX_LINE] = "test.txt";
    char master_server_address[16] = "127.0.0.1";
    while((option = getopt(argc, argv, "r:a:")) != -1) {
        switch(option){
            case 'r': // Request type
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
    client_function(request_type, filename, master_server_address);
}
