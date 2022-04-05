#ifndef MIDDLEWARE_QUEUE_H
#define MIDDLEWARE_QUEUE_H

struct queue_node {
    struct queue_node* next;
    int *client_socket;
};
typedef struct queue_node node_t;

void enqueue(int* client_socket);
int* dequeue();

void show_queue();

#endif //MIDDLEWARE_QUEUE_H
