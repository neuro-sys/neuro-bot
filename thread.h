#ifndef __THREAD_H_
#define __THREAD_H_

typedef void * thread_t ;

int thread_create(thread_t * thread, void *(*start_routine)(void *), void *arg);
int thread_kill(thread_t * thread);

#endif // __THREAD_H_
