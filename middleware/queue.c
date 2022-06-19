#include "queue.h"

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
        new_node->connection->request_type = connection->request_type;
        new_node->connection->client_port = connection->client_port;
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
void enqueue_master_connection(int *socket_descriptor, int request_type, char *client_ip, int client_port) {
    client_connection *connection = malloc(sizeof(client_connection));
    connection->request_type = request_type;
    connection->client_port = client_port;
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
    result.connection->request_type = temp->connection->request_type;
    result.connection->client_port = temp->connection->client_port;
    result.connection->client_ip = malloc(15);
    strcpy(result.connection->client_ip, temp->connection->client_ip);
    free(temp->connection);
    free(temp);
    return result;
}
