#ifndef __QUEUE_H
#define __QUEUE_H

struct queue_t {
    void **data;
    int front;
    int count; 
    int is_full;
    int max_size;
};

#endif

