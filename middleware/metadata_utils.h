#ifndef MIDDLEWARE_METADATA_UTILS_H
#define MIDDLEWARE_METADATA_UTILS_H

#include <uuid/uuid.h>

typedef struct RS {
    int cpu;
    int ram;
    int gpu;
} Resource;

typedef struct MD {
    Resource resources;
    char uuid[UUID_STR_LEN];
} Metadata;

struct list_node {
    struct list_node* next;
    Metadata *worker_metadata;
};
typedef struct list_node metadata_node;

Resource get_local_resources();
void generate_uuid(char *out);
char *metadata_to_str(Metadata metadata);
Metadata str_to_metadata(const char *str);

void add_to_list(Metadata worker_metadata);
void show_list();

#endif //MIDDLEWARE_METADATA_UTILS_H
