#ifndef MIDDLEWARE_QUEUE_H
#define MIDDLEWARE_QUEUE_H

struct connection {
    int request_id;
    char *client_ip;
};
typedef struct connection client_connection;

struct queue_node {
    struct queue_node* next;
    int *socket_descriptor;
    client_connection *connection;
};
typedef struct queue_node node_t;

void enqueue_worker_connection(int* socket_descriptor);
int* dequeue_worker_connection();

void enqueue_client_connection(int* socket_descriptor);
int* dequeue_client_connection();

void enqueue_master_connection(int* socket_descriptor, int request_id, char *client_ip);
node_t dequeue_master_connection();

void show_queue();

#endif //MIDDLEWARE_QUEUE_H
