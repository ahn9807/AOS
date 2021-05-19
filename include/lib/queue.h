#ifndef QUEUE_H
#define QUEUE_H

typedef struct _node {
    struct _node *prev_node, *next_node;
    void* data;
} node_t;

typedef struct _queue {
    node_t *head, *tail;
    int size;
} queue_t;

queue_t* init_queue();
void insert_head(queue_t* q, void* data);
void insert_tail(queue_t* q, void* data);
int remove_head(queue_t* q);
int remove_tail(queue_t* q);
int remove_at(queue_t* q, void *data);
void* peek_head(queue_t* q);
void* peek_tail(queue_t* q);
void* get_head(queue_t* q);
void* get_tail(queue_t* q);
int is_empty(queue_t* q);
void destory(queue_t* q);
void debug_queue(queue_t* q);

#endif