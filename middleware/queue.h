#ifndef MIDDLEWARE_QUEUE_H
#define MIDDLEWARE_QUEUE_H

struct queue_node {
    struct queue_node* next;
    int *socket_descriptor;
};
typedef struct queue_node node_t;

void enqueue_worker_connection(int* socket_descriptor);
int* dequeue_worker_connection();

void enqueue_client_connection(int* socket_descriptor);
int* dequeue_client_connection();

void show_queue();

#endif //MIDDLEWARE_QUEUE_H
