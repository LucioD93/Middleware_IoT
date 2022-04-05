#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <uuid/uuid.h>
#include "metadata_utils.h"

metadata_node* metadata_head = NULL;


int count_files_in_dir_by_name(const char *directory, const char *name) {
    int count = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(name, dir->d_name, strlen(name)) == 0) {
                count++;
            }
        }
        closedir(d);
    }
    return(count);
}


int read_int_from_file(const char* file_name) {
    FILE* file = fopen (file_name, "r");
    int value = 0;

    fscanf (file, "%d", &value);
    fclose (file);

    return value;
}


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


Resource get_local_resources() {

    Resource local_resources;

    int block_size_bytes = read_int_from_file("/sys/devices/system/memory/block_size_bytes");

    // Deduct two from CPU count because of cpufreq and cpuidle files
    local_resources.cpu = get_cpu_score();
    local_resources.ram = get_ram_score();
    local_resources.gpu = 3;

    return local_resources;
}

void generate_uuid(char *out) {
    uuid_t uuid;
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, out);
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
            "%d,%d,%d,%s",
            metadata.resources.cpu, metadata.resources.ram, metadata.resources.gpu, metadata.uuid
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
            "%d,%d,%d,%s",
            metadata.resources.cpu, metadata.resources.ram, metadata.resources.gpu, metadata.uuid
        ) > len + 1
    )
    {
        fprintf (stderr, "%s() error: snprintf returned truncated result.\n", __func__);
        return NULL;
    }

    return apstr;
}


Metadata str_to_metadata(const char *str) {
    Metadata metadata;
    sscanf(
            str,
            "%d,%d,%d,%s",
            &metadata.resources.cpu, &metadata.resources.ram, &metadata.resources.gpu, metadata.uuid
    );

    return metadata;
}


void add_to_list(Metadata worker_metadata) {
    metadata_node *current = metadata_head;
    while (current != NULL && current->next != NULL) {
        if (strcmp(current->worker_metadata->uuid, worker_metadata.uuid) == 0) {
            printf("FOUND REPETITION %s\n", current->worker_metadata->uuid);
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
    
    new_node->next = metadata_head;
    
    metadata_head = new_node;
}


void show_list() {
    printf("Metadata list\n------------------\n");
    metadata_node *node = metadata_head;
    while (node != NULL) {
        printf(
            "ID: %s\n   CPU %d - RAM %d, GPU %d\n",
            node->worker_metadata->uuid,
            node->worker_metadata->resources.cpu,
            node->worker_metadata->resources.ram,
            node->worker_metadata->resources.gpu
        );
        node = node->next;
    }
    printf("---------\n");
}