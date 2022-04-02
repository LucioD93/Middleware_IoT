#include <stdio.h>
#include "metadata_utils.h"
#include "threading_utils.h"

Metadata create_worker_metadata() {
    Metadata worker_metadata;

    generate_uuid(worker_metadata.uuid);
    worker_metadata.resources = get_local_resources();

    return worker_metadata;
}

int main(void) {


    Metadata worker_metadata = create_worker_metadata();
    printf("WORKER UUID [%s]\n", worker_metadata.uuid);
    printf("CPU %d\nRAM %d\n", worker_metadata.resources.cpu, worker_metadata.resources.ram);

    worker(worker_metadata);
}
