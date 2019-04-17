
#include "queue.h"

void push(queue *q, uint value)
{
    q->data[q->put_index++] = value;
    q->put_index %= MAX_USERS;
}

uint pop(queue *q)
{
    uint value = q->data[q->get_index++];
    q->get_index %= MAX_USERS;
    return value;
}

bool empty(queue *q)
{
    return q->get_index == q->put_index;
}
