#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

node_t* head = NULL;
node_t* tail = NULL;


// Add new socket to the queue
void enqueue(int *client_socket) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->client_socket = client_socket;
    new_node->next = NULL;
    if (tail == NULL) {
        head = new_node;
    } else {
        tail->next = new_node;
    }
    tail = new_node;
}


// Get first node in the queue, return NULL if empty
int* dequeue() {
    if (head == NULL) {
        return NULL;
    } else {
        int *result = head->client_socket;
        node_t *temp = head;
        head = head->next;
        if (head == NULL) {
            tail = NULL;
        }
        free(temp);
        return result;
    }
}


void show_queue() {
    node_t *temp = head;
    printf("QUEUE: ");
    while (temp != NULL) {
        printf("%d->", *temp->client_socket);
        temp = temp->next;
    }
    printf("NULL\n");
}
