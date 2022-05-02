#include "client_utils.h"

_Noreturn void client_thread_thread() {
    int sockfd, sendbytes;
    SA_IN servaddr;
    char sendline[MAXLINE];

    check(
            (sockfd = socket(AF_INET, SOCK_STREAM, 0)),
            "Client socket creation failed"
    );

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(CLIENTS_PORT);

    check(
            (inet_pton(AF_INET, SERVERADDRESS, &servaddr.sin_addr)),
            "Server address translation failed"
    );

    check(
            (connect(sockfd, (SA *) &servaddr, sizeof(servaddr))),
            "Connection failed"
    );

    printf("CONNECTED\n");

    strcpy(sendline, "SENDING FROM CLIENT");
    sendbytes = sizeof(sendline);

    printf("SENDING:[%d][%s]\n", sendbytes, sendline);

    check(
            (write(sockfd, &sendline, sendbytes) != sendbytes),
            "Socket write failed"
    );

    check(close(sockfd), "Socket closing Failed");

    printf("Socket closed\n");
    exit(0);

}