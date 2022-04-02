#ifndef MIDDLEWARE_QUEUE_H
#define MIDDLEWARE_QUEUE_H

struct node {
    struct node* next;
    int *client_socket;
};
typedef struct node node_t;

void enqueue(int* client_socket);
int* dequeue();

void show_queue();

#endif //MIDDLEWARE_QUEUE_H
