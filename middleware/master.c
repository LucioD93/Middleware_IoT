#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include "metadata_utils.h"
#include "queue.h"
#include "threading_utils.h"


int main(void) {

    char *uuid = malloc(UUID_STR_LEN);
    generate_uuid(uuid);
    printf("SLAVE UUID [%s]\n", uuid);

    Resource *r = get_local_resources("1");


    printf("CPU %d\nRAM %d\n", r->cpu, r->ram);

    master_server();

    int x[5] = {1,2,3,4,5};
    for (int i = 0; i < 5; i++) {
        enqueue(&x[i]);
    }
    show_queue();

    int *j;
    for (int i = 0; i < 5; i++) {
        j = dequeue();
        printf("GOT %d\n", *j);
        enqueue(&x[i]);
        show_queue();
    }

    j = dequeue();
    printf("GOT %d\n", *j);
    show_queue();

    return 0;
}
