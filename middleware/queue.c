#include "queue.h"
#include <stdlib.h>
#include <string.h>

node_t* worker_queue_head = NULL;
node_t* worker_queue_tail = NULL;

node_t* client_queue_head = NULL;
node_t* client_queue_tail = NULL;

node_t* master_queue_head = NULL;
node_t* master_queue_tail = NULL;


void enqueue(node_t** queue_head, node_t** queue_tail, int *socket_descriptor, client_connection *connection) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->socket_descriptor = socket_descriptor;
    if (connection != NULL) {
        new_node->connection = malloc(sizeof(client_connection));
        new_node->connection->request_id = connection->request_id;
        new_node->connection->client_ip = malloc(15);
        strcpy(new_node->connection->client_ip, connection->client_ip);
    }
    new_node->next = NULL;
    if (*queue_tail == NULL) {
        *queue_head = new_node;
    } else {
        (*queue_tail)->next = new_node;
    }
    *queue_tail = new_node;
}

node_t* dequeue(node_t** queue_head, node_t** queue_tail) {
    if (queue_head == NULL) {
        return NULL;
    } else {
        node_t *result = *queue_head;
        *queue_head = (*queue_head)->next;
        if (*queue_head == NULL) {
            *queue_tail = NULL;
        }
        return result;
    }
}

// Add new socket to the queue
void enqueue_worker_connection(int *socket_descriptor) {
    enqueue(&worker_queue_head, &worker_queue_tail, socket_descriptor, NULL);
}

// Get first node in the queue, return NULL if empty
int* dequeue_worker_connection() {
    node_t *temp = dequeue(&worker_queue_head, &worker_queue_tail);
    int* socket_descriptor = temp->socket_descriptor;
    free(temp);
    return  socket_descriptor;
}


// Add new socket to the queue
void enqueue_client_connection(int *socket_descriptor) {
    enqueue(&client_queue_head, &client_queue_tail, socket_descriptor, NULL);
}

// Get first node in the queue, return NULL if empty
int* dequeue_client_connection() {
    node_t *temp = dequeue(&client_queue_head, &client_queue_tail);
    int* socket_descriptor = temp->socket_descriptor;
    free(temp);
    return  socket_descriptor;
}


// Add new socket to the queue
void enqueue_master_connection(int *socket_descriptor, int request_id, char *client_ip) {
    client_connection *connection = malloc(sizeof(client_connection));
    connection->request_id = request_id;
    connection->client_ip = malloc(15);
    strcpy(connection->client_ip, client_ip);
    enqueue(&master_queue_head, &master_queue_tail, socket_descriptor, connection);
    free(connection);
}

// Get first node in the queue, return NULL if empty
node_t dequeue_master_connection() {
    node_t *temp = dequeue(&master_queue_head, &master_queue_tail);
    node_t result;
    result.socket_descriptor = temp->socket_descriptor;
    result.connection = malloc(sizeof(client_connection));
    result.connection->request_id = temp->connection->request_id;
    result.connection->client_ip = malloc(15);
    strcpy(result.connection->client_ip, temp->connection->client_ip);
    free(temp->connection);
    free(temp);
    return result;
}
