#ifndef MIDDLEWARE_METADATA_UTILS_H
#define MIDDLEWARE_METADATA_UTILS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <uuid/uuid.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include "threading_utils.h"
#include "constants.h"

typedef struct RS {
    int cpu;
    int ram;
    int gpu;
    int cpu_usage;
    int max_tasks;
    int estimated_tasks;
    int assigned_tasks;
    long long network_delay;
} Resource;

typedef struct MD {
    Resource resources;
    char uuid[UUID_STR_LEN];
} Metadata;

typedef struct list_node {
    struct list_node* next;
    int worker_socket;
    Metadata *worker_metadata;
    pthread_mutex_t worker_selection_mutex;
} metadata_node;

typedef struct s_sleep_args {
    int *tasks_tracker;
    int milliseconds;
} sleep_args;

int get_cpu_usage();

Resource get_local_resources();
Metadata create_worker_metadata();
void generate_uuid(char *out);
char *metadata_to_str(Metadata metadata);
Metadata str_to_metadata(const char *str);

void add_to_list(Metadata worker_metadata, int worker_socket);
void print_metadata(Metadata metadata);
void remove_from_list(char* uuid);
void show_worker_list();

metadata_node *select_worker(int request_type);
long long milliseconds_since_epoch();

void sleep_for_nanoseconds(long nanoseconds);
void sleep_for_milliseconds(long milliseconds);

#endif //MIDDLEWARE_METADATA_UTILS_H
