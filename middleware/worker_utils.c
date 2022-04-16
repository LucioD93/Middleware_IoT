#include "worker_utils.h"

_Noreturn void worker_metadata_thread() {
    Metadata worker_metadata = create_worker_metadata();
    printf("WORKER UUID [%s]\n", worker_metadata.uuid);
    printf("CPU %d - RAM %d - GPU %d\n", worker_metadata.resources.cpu, worker_metadata.resources.ram, worker_metadata.resources.gpu);

    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAXLINE];
    char *metadata_str;

    check(
            (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
            "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVERPORT);

    check(
            (inet_pton(AF_INET, SERVERADDRESS, &servaddr.sin_addr)),
            "Server address translation failed"
    );

    check(
            (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
            "Connection failed"
    );

    printf("CONNECTED\n");

    while(keep_running == 1) {
        worker_metadata.resources.cpu_usage = get_cpu_usage();
        metadata_str = metadata_to_str(worker_metadata);
        strcpy(sendline, metadata_str);
        sendbytes = sizeof(sendline);
        free(metadata_str);

        printf("SENDING:[%d][%s]\n", sendbytes, sendline);

        check(
                (write(sockfd, &sendline, sendbytes) != sendbytes),
                "Socket write failed"
        );
        sleep(5);
    }

    check(close(sockfd), "Socket closing Failed");

    printf("Socket closed\n");
    exit(0);

}


void worker() {
    worker_metadata_thread();
}
