#ifndef QUEUE_H
#define QUEUE_H

typedef struct _queue_noe {
    struct _queue_noe *prev_node, *next_node;
    void* data;
} queue_node_t;

typedef struct _queue {
    queue_node_t *head, *tail;
    int size;
} queue_t;

queue_t* queue_init();
void queue_insert_head(queue_t* q, void* data);
void queue_insert_tail(queue_t* q, void* data);
int queue_remove_head(queue_t* q);
int queue_remove_tail(queue_t* q);
int queue_remove_at(queue_t* q, void *data);
void* queue_peek_head(queue_t* q);
void* queue_peek_tail(queue_t* q);
void* queue_get_head(queue_t* q);
void* queue_get_tail(queue_t* q);
int queue_is_empty(queue_t* q);
void queue_free(queue_t* q);
void debug_queue(queue_t* q);

#endif