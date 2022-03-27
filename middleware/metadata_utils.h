#ifndef MIDDLEWARE_METADATA_UTILS_H
#define MIDDLEWARE_METADATA_UTILS_H

typedef struct Resources {
    int cpu;
    int ram;
    int gpu;
} Resource;

Resource *get_local_resources(char *id);
void generate_uuid(char *out);

#endif //MIDDLEWARE_METADATA_UTILS_H
