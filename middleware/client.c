#include "client_utils.h"


void print_usage() {
    printf("-r [1-6]   Tipo de peticion\n");
    printf("      1      Procesamiento de imagen\n");
    printf("      2      Consulta web\n");
    printf("      3      Procesamiento de palabras\n");
    printf("      4      Sincronizacion\n");
    printf("      5      Localizacion por imagen\n");
    printf("      6      Localizacion por direccion IP\n");
    printf("      7      Seleccion aleatoria\n");
    printf("-a [ip]    Direccion IP de servidor maestro\n");
}


typedef struct CA {
    int request_type;
    char master_server_address[16];
    char client_address[16];
} client_args;


void *client_thread_function(void *args) {
    client_args actual_args = *((client_args *)args);
    int request_type = actual_args.request_type;
    char *master_server_address = actual_args.master_server_address;
    char *client_address = actual_args.client_address;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);

    client_function(request_type, master_server_address, client_address);
    
    gettimeofday(&end, NULL);
    float delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    // printf("%f\n",delta);
    float *time = (float *)malloc(sizeof(float));
    *time = delta;
    return time;
}


int main(int argc, char *argv[]) {
    int option, request_type, number_of_requests = 1;
    char master_server_address[16] = DEFAULT_MASTER_SERVER_ADDRESS;
    char client_address[16] = DEFAULT_MASTER_SERVER_ADDRESS;

    while((option = getopt(argc, argv, "r:a:n:i:")) != -1) {
        switch(option) {
        case 'n': // Number of requests
            number_of_requests = atoi(optarg);
            break;
        case 'r': // Request type
            request_type = atoi(optarg);
            break;
        case 'a': // Master server address
            strcpy(master_server_address, optarg);
            break;
        case 'i': // Client address
            strcpy(client_address, optarg);
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

    if (request_type < 1 || request_type > 7) {
        printf("Error! Tipo de peticion debe estar entre 1 y 6\n");
        exit(1);
    }
    if (!isValidIpAddress(master_server_address)) {
        printf("Invalid IP address for master server\n");
        exit(1);
    }
    if (!isValidIpAddress(client_address)) {
        printf("Invalid IP address for client\n");
        exit(1);
    }
    
    pthread_t clients_thread_pool[number_of_requests];
    client_args *args_for_client[number_of_requests];

    float *times[number_of_requests];

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < number_of_requests; i++) {
        args_for_client[i] = malloc(sizeof(client_args));
        args_for_client[i]->request_type = request_type;
        strcpy(args_for_client[i]->master_server_address, master_server_address);
        strcpy(args_for_client[i]->client_address, client_address);
        pthread_create(
            &clients_thread_pool[i],
            NULL,
            client_thread_function,
            args_for_client[i]
        );
        sleep_for_nanoseconds(10);
    }

    for (int i = 0; i < number_of_requests; i++) {
        pthread_join(clients_thread_pool[i], (void *)&times[i]);
        free(args_for_client[i]);
    }

    gettimeofday(&end, NULL);
    float delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    printf("Total time taken: %f\n",delta);

    float sum = 0;
    float min = FLT_MAX;
    float max = FLT_MIN;
    for (int i = 0; i < number_of_requests; i++) {
        if (*times[i] < min) {
            min = *times[i];
        }
        if (*times[i] > max) {
            max = *times[i];
        }
        sum += *times[i];
    }
    float average = sum / number_of_requests;

    printf("Min: %9.6f\nMax: %9.6f\nAverage: %9.6f\n\n", min, max, average);
    for (int i = 0; i < number_of_requests; i++) {
        free(times[i]);
    }
}
