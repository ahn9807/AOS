/**
 * Queue is implemented by linked list (like linux...)
 * Each mthread_tcb has perv and next pointer.
 * We can traverse queue start from given queue
 *
 * Implemented by junho ahn at NOV 12 2020
 */
#include "queue.h"
#include "vga_text.h"
#include "kmalloc.h"
#include <stdlib.h>

queue_t* init_queue() {
    queue_t* q = kmalloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

void insert_head(queue_t* q, void* in) {
    node_t* n = kmalloc(sizeof(node_t));
    n->data = in;
    n->next_node = NULL;
    n->prev_node = q->head;
    if(q->head != NULL) {
        q->head->next_node = n;
    }
    q->head = n;
    q->size = q->size + 1;

    if(q->tail == NULL) {
        q->tail = q->head;
    }
}

void insert_tail(queue_t* q, void* in) {
    node_t* n = kmalloc(sizeof(node_t));
    n->data = in;
    n->prev_node = NULL;
    n->next_node = q->tail;
    if(q->tail != NULL) {
        q->tail->prev_node = n;
    }
    q->tail = n;
    q->size = q->size + 1;

    if(q->head == NULL) {
        q->head = q->tail;
    }
}

int remove_head(queue_t* q) {
    node_t* temp_n = q->head;
    if(temp_n == NULL) {
        return -1;
    }

    if(q->head == q->tail) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = q->head->prev_node;
    }

    kfree(temp_n);
    q->size = q->size -1;
}

int remove_tail(queue_t* q) {
    node_t* temp_node = q->tail;
    if(temp_node == NULL) {
        return -1;
    }

    if(q->head == q->tail) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->tail = q->tail->prev_node;
    }


    kfree(temp_node);
    q->size = q->size -1;
}

int remove_at(queue_t *q, void* tcb) {
    if(q == NULL || tcb == NULL) {
        return -1;
    }

    node_t* iter = q->head;

    while(iter != NULL) {
        if(iter->data == tcb) {
            node_t* next_node = iter->next_node;
            node_t* prev_node = iter->prev_node;

            prev_node->next_node = next_node;
            next_node->prev_node = prev_node;

            kfree(iter);

            break;
        }
        iter = iter->next_node;
    }
    q->size = q->size -1;

    return 0;

}

void* peek_head(queue_t* q) {
    return q->head->data;
}

void* peek_tail(queue_t* q) {
    return q->tail->data;
}

void* get_head(queue_t* q) {
    void* return_val;
    return_val = peek_head(q);
    remove_head(q);
    return return_val;
}

void* get_tail(queue_t* q) {
    void* return_val;
    return_val = peek_tail(q);
    remove_tail(q);
    return return_val;
}

int is_empty(queue_t* q) {
    return q->size == 0 ? 1 : 0;
}

void destory(queue_t* q) {
    if(q == NULL) {
        return;
    }

    node_t* iter = q->head;

    while(iter != NULL) {
        node_t* cur_node = iter;
        iter = iter->next_node;
        kfree(cur_node);
    }

    kfree(q);
}

void debug_queue(queue_t* q) {
    printf("=== Queue ===\n");
    printf("Size: %d, Head: %p, Tail:%p\n", q->size, q->head, q->tail);
    node_t* iter = q->head;
    for(int i=0;i<q->size;i++) {
        printf("Element: %p\n", iter);
        iter = iter->prev_node;
    }
}