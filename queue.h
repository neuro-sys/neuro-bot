#ifndef __QUEUE_H
#define __QUEUE_H

#define MAX_QUEUE_SIZE 500

struct queue_t {
    void * data[500];
    int front;
    int count; 
    int is_full;
};

#endif

