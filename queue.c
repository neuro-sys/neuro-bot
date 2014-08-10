#include "queue.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

void queue(struct queue_t * queue, void * data)
{
    assert(queue != NULL);

    if (queue->is_full) {
        fprintf(stderr, "queue is full.\n");
        return;
    }

    queue->data[queue->count++] = data;

    if (queue->count >= queue->max_size)
        queue->count = 0;

    if (queue->count == queue->front)
        queue->is_full = 1;
}

void * unqueue(struct queue_t * queue)
{
    void * data;

    assert(queue != NULL);

    if (!queue->is_full && queue->count == queue->front) {
        fprintf(stderr, "queue is empty.\n");
        return NULL;
    }

    data = queue->data[queue->front++];

    if (queue->front >= queue->max_size)
        queue->front = 0;

    queue->is_full = 0;

    return data;
}

#ifdef TEST_QUEUE
int main(int argc, char *argv[])
{
    struct queue_t queue_test;
    memset(&queue_test, 0, sizeof(queue_test));

    queue_test.max_size = 500;

    queue_test.data = malloc(queue_test.max_size);

    queue(&queue_test, "elma");
    queue(&queue_test, "armut");
    queue(&queue_test, "mandalina");
    queue(&queue_test, "sebze");

    {
        void * data;

        while ((data = unqueue(&queue_test)) != NULL) {
            puts(data);
        }
    }

    return 0;
}

#endif


