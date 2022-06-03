#include "worker_utils.h"

void term() {
    printf("Exit with ctrl + Z!\n");
}

void print_usage() {
    printf("-a [ip]    Direccion IP de servidor maestro\n");
}

int main(int argc, char *argv[]) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGINT, &action, NULL);

    int option;
    char master_server_address[16] = "127.0.0.1";
    while((option = getopt(argc, argv, "r:a:")) != -1) {
        switch(option){
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

    if (!isValidIpAddress(master_server_address)) {
        printf("Invalid IP address for master server\n");
        exit(1);
    }

    worker(master_server_address);
}
