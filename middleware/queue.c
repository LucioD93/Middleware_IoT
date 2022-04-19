#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

node_t* worker_queue_head = NULL;
node_t* worker_queue_tail = NULL;

node_t* client_queue_head = NULL;
node_t* client_queue_tail = NULL;


void enqueue(node_t** queue_head, node_t** queue_tail, int *socket_descriptor) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->socket_descriptor = socket_descriptor;
    new_node->next = NULL;
    if (*queue_tail == NULL) {
        *queue_head = new_node;
    } else {
        (*queue_tail)->next = new_node;
    }
    *queue_tail = new_node;
}

int* dequeue(node_t** queue_head, node_t** queue_tail) {
    if (queue_head == NULL) {
        return NULL;
    } else {
        int *result = (*queue_head)->socket_descriptor;
        node_t *temp = *queue_head;
        *queue_head = (*queue_head)->next;
        if (*queue_head == NULL) {
            *queue_tail = NULL;
        }
        free(temp);
        return result;
    }
}

// Add new socket to the queue
void enqueue_worker_connection(int *socket_descriptor) {
    printf("QUEUE\n");
    enqueue(&worker_queue_head, &worker_queue_tail, socket_descriptor);
}


// Get first node in the queue, return NULL if empty
int* dequeue_worker_connection() {
    printf("DEQUEUE\n");
    return dequeue(&worker_queue_head, &worker_queue_tail);
}
