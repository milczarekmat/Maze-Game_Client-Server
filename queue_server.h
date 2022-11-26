#ifndef GRA_PROJEKT_QUEUE_SERVER_H
#define GRA_PROJEKT_QUEUE_SERVER_H
typedef struct node_t NODE;
typedef struct queue_t QUEUE;

struct node_t{
    int* client_socket;
    NODE* next;
};

struct queue_t{
    NODE* head;
    NODE* tail;
};

QUEUE *create_queue();
void enqueue(int* client_socket, QUEUE *queue);
int* dequeue(QUEUE *queue);
#endif
