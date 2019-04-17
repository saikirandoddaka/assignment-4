#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#include "config.h"

typedef struct queue_struct {
    uint data[MAX_USERS];
    uint put_index;
    uint get_index;
} queue;

void push(queue *q, uint value);
uint pop(queue *q);
bool empty(queue *q);


#endif // QUEUE_H
