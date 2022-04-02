#ifndef MIDDLEWARE_METADATA_UTILS_H
#define MIDDLEWARE_METADATA_UTILS_H

#include <uuid/uuid.h>

typedef struct Resources {
    int cpu;
    int ram;
    int gpu;
} Resource;

typedef struct MD {
    Resource resources;
    char uuid[UUID_STR_LEN];
} Metadata;

Resource get_local_resources();
void generate_uuid(char *out);

#endif //MIDDLEWARE_METADATA_UTILS_H
