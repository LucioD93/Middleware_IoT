#include "metadata_utils.h"

metadata_node* metadata_head = NULL;


int get_cpu_score() {
    int processors = 0;
    int cores = 0;
    int threads = 0;
    int clock = 0;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if (strncmp(line, "processor", strlen("processor")) == 0) {
            processors++;
        } else if (strncmp(line, "cpu cores", strlen("cpu cores")) == 0) {
            sscanf(line, "cpu cores       : %d", &cores);
            threads += cores;
        } else if (clock ==0 && strncmp(line, "cpu MHz", strlen("cpu MHz")) == 0) {
            sscanf(line, "cpu MHz         : %d", &clock);
        }

    }

    fclose(fp);
    if (line) free(line);

    int score = processors*threads*clock/1000;
    if ((score - 83.2)*(score-1) <= 0) {
        return 1;
    }
    if ((score - 90)*(score-83.2) <= 0) {
        return 2;
    }
    if ((score - 100)*(score-90) <= 0) {
        return 3;
    }
    if ((score - 114.9)*(score-100) <= 0) {
        return 4;
    }
    if ((score - 281)*(score-115) <= 0) {
        return 5;
    }
    if ((score - 700)*(score-281) <= 0) {
        return 6;
    }
    if ((score - 2000)*(score-700) <= 0) {
        return 7;
    }
    if ((score - 6000)*(score-2000) <= 0) {
        return 8;
    }
    if ((score - 10000)*(score-6000) <= 0) {
        return 9;
    }
    return 10;
}


int get_ram_score() {
    int ram_size_kb = 0;
    int score = 0;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if (strncmp(line, "MemTotal", strlen("MemTotal")) == 0) {
            sscanf(line, "MemTotal:        %d kB", &ram_size_kb);
            score = ram_size_kb / 1000000;
            break;
        }
    }

    fclose(fp);
    if (line) free(line);

    if ((score - 2)*(score) <= 0) {
        return 1;
    }
    if ((score - 4)*(score-2) <= 0) {
        return 2;
    }
    if ((score - 6)*(score-4) <= 0) {
        return 3;
    }
    if ((score - 8)*(score-6) <= 0) {
        return 4;
    }
    if ((score - 16)*(score-8) <= 0) {
        return 5;
    }
    if ((score - 32)*(score-16) <= 0) {
        return 6;
    }
    if ((score - 64)*(score-32) <= 0) {
        return 7;
    }
    if ((score - 100)*(score-64) <= 0) {
        return 8;
    }
    if ((score - 128)*(score-100) <= 0) {
        return 9;
    }
    return 10;

}


int get_cpu_usage() {
    float load;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/proc/loadavg", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    read = getline(&line, &len, fp);
    if (read == -1)
        exit(EXIT_FAILURE);
    sscanf(line, "%f", &load);
    fclose(fp);
    return (int)(load * 100);
}


int get_max_task_unbound(Resource server) {
    float cpu = server.cpu * ALPHA;
    float ram = server.cpu * BETA;
    float gpu = server.cpu * GAMMA;

    return round(pow(cpu + ram + gpu, 2));
}


int get_max_tasks(Resource server) {
    Resource max_server = {10,10,10};

    int max_value = get_max_task_unbound(max_server);
    float value = max_value / THREAD_POOL_SIZE;
    return round(get_max_task_unbound(server)/value) + 1;
}


Resource get_local_resources() {

    Resource local_resources;

    local_resources.cpu = get_cpu_score();
    local_resources.ram = get_ram_score();
    local_resources.gpu = 3;
    local_resources.cpu_usage = get_cpu_usage();
    local_resources.max_tasks = get_max_tasks(local_resources);
    local_resources.assigned_tasks = 0;

    return local_resources;
}


void generate_uuid(char *out) {
    uuid_t uuid;
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, out);
}


Metadata create_worker_metadata() {
    Metadata worker_metadata;

    generate_uuid(worker_metadata.uuid);
    worker_metadata.resources = get_local_resources();

    return worker_metadata;
}


/* convert contents of metadata structure to a comma
   separated string of values. Returns pointer to
   dynamically allocated string containing contents
   of metadata structure on success, otherwise NULL.
*/
char *metadata_to_str(Metadata metadata) {
    /* get length of string required to hold struct values */
    size_t len = 0;
    char *dummy = malloc(0);
    len = snprintf (
        dummy,
        len,
        "%d,%d,%d,%d,%d,%d,%d,%lld,%s\n",
        metadata.resources.cpu,
        metadata.resources.ram,
        metadata.resources.gpu,
        metadata.resources.cpu_usage,
        metadata.resources.max_tasks,
        metadata.resources.assigned_tasks,
        metadata.resources.estimated_tasks,
        milliseconds_since_epoch(),
        metadata.uuid
    );
    free(dummy);

    /* allocate/validate string to hold all values (+1 to null-terminate) */
    char *apstr = calloc (1, sizeof *apstr * len + 1);
    if (!apstr) {
        fprintf (stderr, "%s() error: virtual memory allocation failed.\n", __func__);
    }

    /* write/validate struct values to apstr */
    if (
        snprintf (
            apstr,
            len + 1,
            "%d,%d,%d,%d,%d,%d,%d,%lld,%s\n",
            metadata.resources.cpu,
            metadata.resources.ram,
            metadata.resources.gpu,
            metadata.resources.cpu_usage,
            metadata.resources.max_tasks,
            metadata.resources.assigned_tasks,
            metadata.resources.estimated_tasks,
            milliseconds_since_epoch(),
            metadata.uuid
        ) > len + 1
    )
    {
        fprintf (stderr, "%s() error: snprintf returned truncated result.\n", __func__);
        return NULL;
    }

    return apstr;
}


Metadata str_to_metadata(const char *str) {
    long long milliseconds_received;
    Metadata metadata;
    sscanf(
        str,
        "%d,%d,%d,%d,%d,%d,%d,%lld,%s\n",
        &metadata.resources.cpu,
        &metadata.resources.ram,
        &metadata.resources.gpu,
        &metadata.resources.cpu_usage,
        &metadata.resources.max_tasks,
        &metadata.resources.assigned_tasks,
        &metadata.resources.estimated_tasks,
        &milliseconds_received,
        metadata.uuid
    );

    metadata.resources.network_delay = milliseconds_since_epoch() - milliseconds_received;
    return metadata;
}


void add_to_list(Metadata worker_metadata, int worker_socket) {
    metadata_node *current = metadata_head;
    while (current != NULL) {
        if (memcmp(current->worker_metadata->uuid, worker_metadata.uuid, UUID_STR_LEN) == 0) {
            current->worker_metadata->resources.cpu_usage = worker_metadata.resources.cpu_usage;
            current->worker_metadata->resources.assigned_tasks = worker_metadata.resources.assigned_tasks;
            current->worker_metadata->resources.estimated_tasks = worker_metadata.resources.assigned_tasks;
            return;
        }
        current = current->next;
    }
    metadata_node *new_node = malloc(sizeof(metadata_node));
    
    new_node->worker_metadata = malloc(sizeof(Metadata));
    strcpy(new_node->worker_metadata->uuid, worker_metadata.uuid);
    new_node->worker_metadata->resources.cpu = worker_metadata.resources.cpu;
    new_node->worker_metadata->resources.ram = worker_metadata.resources.ram;
    new_node->worker_metadata->resources.gpu = worker_metadata.resources.gpu;
    new_node->worker_metadata->resources.cpu_usage = worker_metadata.resources.cpu_usage;
    new_node->worker_metadata->resources.max_tasks = worker_metadata.resources.max_tasks;
    new_node->worker_metadata->resources.assigned_tasks = worker_metadata.resources.assigned_tasks;
    new_node->worker_metadata->resources.estimated_tasks = 0;
    new_node->worker_metadata->resources.network_delay = worker_metadata.resources.network_delay;
    new_node->worker_socket = worker_socket;
    new_node->next = metadata_head;
    
    metadata_head = new_node;
}


void remove_from_list(char* uuid) {
    printf("Deleting %s\n", uuid);
    metadata_node *current = metadata_head, *prev;

    if(current != NULL && memcmp(current->worker_metadata->uuid, uuid, UUID_STR_LEN) == 0) {
        metadata_head = current->next;
        free(current);
        return;
    }

    while (current != NULL && memcmp(current->worker_metadata->uuid, uuid, UUID_STR_LEN) == 0) {
        prev = current;
        current = current->next;
    }

    if(current != NULL) {
        prev->next = current->next;
        free(current);
    }
    printf("REMOVED %s\n", uuid);
    show_list();
}


Resource resources_per_request_id(int request_id) {
    Resource result;
    switch (request_id) {
        case IMAGE_PROCESSING_REQUEST:
            result.cpu = 6;
            result.ram = 5;
            result.gpu = 6;
            break;
        case WEB_REQUEST:
            result.cpu = 6;
            result.ram = 4;
            result.gpu = 4;
            break;
        case WORD_PROCESSING_REQUEST:
            result.cpu = 6;
            result.ram = 5;
            result.gpu = 2;
            break;
        case SYNCHRONIZATION_REQUEST:
            result.cpu = 1;
            result.ram = 1;
            result.gpu = 1;
            break;
        case IMAGE_LOCATION_REQUEST:
            result.cpu = 6;
            result.ram = 4;
            result.gpu = 5;
            break;
        case IP_LOCATION_REQUEST:
            result.cpu = 3;
            result.ram = 2;
            result.gpu = 1;
            break;
    }
    return result;
}


float worker_apc_for_request_id(int request_id, Resource worker) {
    Resource request_resource = resources_per_request_id(request_id);
    float R = (
        (worker.cpu - request_resource.cpu)*request_resource.cpu +
        (worker.ram - request_resource.ram)*request_resource.ram +
        (worker.gpu - request_resource.gpu)*request_resource.gpu
    );
    float normalized_r = ((R + 75)/113)*10;
    return normalized_r;
}


bool can_resource_process_request(Resource worker) {
    return worker.assigned_tasks < worker.max_tasks && worker.cpu_usage < 90;
}


int estimate_time_per_request_id(int request_id) {
    switch (request_id) {
        case IMAGE_PROCESSING_REQUEST:
            return 600;
        case WEB_REQUEST:
            return 10;
        case WORD_PROCESSING_REQUEST:
            return 100;
        case SYNCHRONIZATION_REQUEST:
            return 10;
        case IMAGE_LOCATION_REQUEST:
            return 300;
        case IP_LOCATION_REQUEST:
            return 30;
    }
    return 999;
}

void *sleeper_function(void *args) {
    sleep_args actual_args = *((sleep_args *)args);
    (*actual_args.tasks_tracker)++;

    long seconds = 0;
    long nanoseconds = estimate_time_per_request_id(actual_args.request_id) * 1000000L;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);

    if(*actual_args.tasks_tracker > 0) (*actual_args.tasks_tracker)--;
}


metadata_node *select_worker(int request_id) {
    metadata_node *current_node = metadata_head;
    metadata_node *max_node = metadata_head;
    float max_apc = worker_apc_for_request_id(request_id, metadata_head->worker_metadata->resources);
    float current_apc;
    while (current_node != NULL) {
        current_apc = worker_apc_for_request_id(request_id, current_node->worker_metadata->resources);
        if (current_apc >= max_apc && can_resource_process_request(current_node->worker_metadata->resources)) {
            max_node = current_node;
        }
        current_node = current_node->next;
    }

    sleep_args *args = malloc(sizeof(sleep_args));
    args->tasks_tracker = &max_node->worker_metadata->resources.estimated_tasks;
    args->request_id = request_id;
    pthread_t sleep_thread;
    pthread_create(
        &sleep_thread,
        NULL,
        sleeper_function,
        args
    );

    return max_node;
}


void print_metadata(Metadata metadata) {
    printf(
        "ID: [%s]\n  CPU %d - RAM %d - GPU %d - CPU Usage %d - Delay %lld - Tasks E: %d(A: %d)(M: %d)\n\n",
        metadata.uuid,
        metadata.resources.cpu,
        metadata.resources.ram,
        metadata.resources.gpu,
        metadata.resources.cpu_usage,
        metadata.resources.network_delay,
        metadata.resources.estimated_tasks,
        metadata.resources.assigned_tasks,
        metadata.resources.max_tasks
    );
}


void show_list() {
    printf("Metadata list\n------------------\n");
    metadata_node *node = metadata_head;
    while (node != NULL) {
        print_metadata(*node->worker_metadata);
        node = node->next;
    }
    printf("---------\n");
}


long long milliseconds_since_epoch() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return te.tv_sec*1000LL + te.tv_usec/1000;
}
