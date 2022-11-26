#include "queue_server.h"
#include <stdlib.h>
#include <stdio.h>

QUEUE *create_queue(){
    QUEUE* new_queue = malloc(sizeof(QUEUE));
    if (!new_queue){
        perror("Failed to allocate new queue");
        exit(1);
    }
    new_queue->head = NULL;
    new_queue->tail = NULL;

    return new_queue;
}

void enqueue(int* client_socket, QUEUE *queue){
    NODE *new_node = malloc(sizeof(NODE));
    if (!new_node){
        perror("Failed with allocation");
        exit(2);
    }
    new_node->next = NULL;
    new_node->client_socket = client_socket;
    if (!queue->head){
        queue->head = new_node;
    }
    else{
        queue->tail->next = new_node;
    }
    queue->tail = new_node;
}

int* dequeue(QUEUE *queue){
    if (!queue->head){
        return NULL;
    }
    else{
        int *result = queue->head->client_socket;
        NODE *temp = queue->head;
        queue->head = queue->head->next;
        if (!queue->head){
            queue->tail = NULL;
        }
        free(temp);
        return result;
    }
}
