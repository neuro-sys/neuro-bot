#include "thread.h"

#include <stdio.h>

#ifdef __WIN32__
    #include <windows.h>
#else
    #include <pthread.h>
    #include <stdlib.h>
    #include <signal.h>
#endif // __WIN32__

int thread_create(thread_t * thread, void *(*start_routine)(void *), void *arg)
{
#ifdef __WIN32__
    void * addr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) start_routine, arg, 0, NULL);
    *thread = addr;
    return 0;
#else
    *thread = malloc(sizeof (pthread_t));
    return pthread_create(*thread, NULL, start_routine, arg);
#endif // __WIN32__
}

int thread_kill(thread_t * thread)
{
#ifdef __WIN32__
    return TerminateThread(*thread, 9);
#else
    pthread_kill(*((pthread_t *) *thread), 9);
    free(*thread);
    return 0;
#endif // __WIN32__
}
