#ifndef MIDDLEWARE_METADATA_UTILS_H
#define MIDDLEWARE_METADATA_UTILS_H

#include <uuid/uuid.h>
#include "constants.h"

typedef struct RS {
    int cpu;
    int ram;
    int gpu;
    int cpu_usage;
} Resource;

typedef struct MD {
    Resource resources;
    char uuid[UUID_STR_LEN];
    int assigned_requests;
} Metadata;

struct list_node {
    struct list_node* next;
    int worker_socket;
    Metadata *worker_metadata;
};
typedef struct list_node metadata_node;

int get_cpu_usage();

Resource get_local_resources();
Metadata create_worker_metadata();
void generate_uuid(char *out);
char *metadata_to_str(Metadata metadata);
Metadata str_to_metadata(const char *str);

void add_to_list(Metadata worker_metadata, int worker_socket);
void print_metadata(Metadata metadata);
void remove_from_list(char* uuid);
void show_list();

metadata_node *select_worker(int request_id);

#endif //MIDDLEWARE_METADATA_UTILS_H
