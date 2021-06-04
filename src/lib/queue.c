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

queue_t* queue_init() {
    queue_t* q = kmalloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

void queue_insert_head(queue_t* q, void* in) {
    queue_node_t* n = kcalloc(1, sizeof(queue_node_t));
    n->data = in;
    n->next_node = q->head;
    n->prev_node = NULL;
    if(q->head != NULL) {
        q->head->prev_node = n;
    }
    q->head = n;
    q->size = q->size + 1;

    if(q->size == 1) {
        q->tail = n;
    }
}

void queue_insert_tail(queue_t* q, void* in) {
    queue_node_t* n = kcalloc(1, sizeof(queue_node_t));
    n->data = in;
    n->prev_node = q->tail;
    n->next_node = NULL;
    if(q->tail != NULL) {
        q->tail->next_node = n;
    }
    q->tail = n;
    q->size = q->size + 1;

    if(q->size == 1) {
        q->head = n;
    }
}

int queue_remove_head(queue_t* q) {
    queue_node_t* temp_n = q->head;
    if(temp_n == NULL) {
        return -1;
    }

    if(q->head == q->tail) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        q->head = q->head->next_node;
    }

    kfree(temp_n);
    q->size = q->size -1;
}

int queue_remove_tail(queue_t* q) {
    queue_node_t* temp_node = q->tail;
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

int queue_remove_at(queue_t *q, void* tcb) {
    if(q == NULL || tcb == NULL) {
        return -1;
    }

    queue_node_t* iter = q->head;

    while(iter != NULL) {
        if(iter->data == tcb) {
            queue_node_t* next_node = iter->next_node;
            queue_node_t* prev_node = iter->prev_node;

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

void* queue_peek_head(queue_t* q) {
    return q->head->data;
}

void* queue_peek_tail(queue_t* q) {
    return q->tail->data;
}

void* queue_get_head(queue_t* q) {
    void* return_val;
    return_val = queue_peek_head(q);
    queue_remove_head(q);
    return return_val;
}

void* queue_get_tail(queue_t* q) {
    void* return_val;
    return_val = queue_peek_tail(q);
    queue_remove_tail(q);
    return return_val;
}

int queue_is_empty(queue_t* q) {
    return q->size == 0 ? 1 : 0;
}

void queue_free(queue_t* q) {
    if(q == NULL) {
        return;
    }

    queue_node_t* iter = q->head;

    while(iter != NULL) {
        queue_node_t* cur_node = iter;
        iter = iter->next_node;
        kfree(cur_node);
    }

    kfree(q);
}

void debug_queue(queue_t* q) {
    printf("=== Queue ===\n");
    printf("Size: %d, Head: %x, Tail:%x\n", q->size, q->head, q->tail);
    queue_node_t* iter = q->head;
    for(int i=0;i<q->size;i++) {
        printf("Element: %c\n", ((char*)iter->data)[0]);
        iter = iter->next_node;
    }
}