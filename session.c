#include "session.h"
#include "socket.h"
#include "irc.h"
#include "global.h"
#include "module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

static pthread_t threads[10];

static void session_init_irc(struct session_t * session)
{
    char message[MAX_IRC_MSG];

    irc_set_nick(session->nickname, message);
    socket_send_message(&session->socket, message);

    irc_set_user("ircbot", "github.com/neuro-sys/neuro-bot", message);
    socket_send_message(&session->socket, message);
}

struct thread_struct {
    struct mod_c_t * mod;
    struct irc_t * irc;
};

static void *looper_thread(void * pdata)
{
    struct thread_struct * ts;

    ts = (struct thread_struct *) pdata;
    ts->mod->func(ts->irc, NULL);

    return 0;
}

static void start_loopers(struct irc_t * irc)
{
    struct mod_c_t ** loopers, ** iterator;
    int i = 0;
    int err;

    loopers = module_get_loopers();
    if (!loopers)
        return;

    iterator = loopers;
    while (*iterator) {
        struct mod_c_t * mod;
        struct thread_struct * ts;

        if (i == 10) {
            fprintf(stderr, "Passed the maximum number of loopers limit.\n");
            break;
        }
        ts = malloc(sizeof (struct thread_struct));

        mod = *iterator++;

        ts->mod = mod;
        ts->irc = irc;

        fprintf(stderr, "Looper %s is started\n", mod->mod_name);
        if ((err = pthread_create(&threads[i++], NULL, looper_thread, ts)) != 0) {
            fprintf(stderr, "Thread could not be started. pthread_create, errno = %d\n", err);
        }
    }
    free(loopers);
}

void session_run(struct session_t * session)
{ 
    char          line[MAX_IRC_MSG];
    struct irc_t  irc;

    memset(&irc, 0, sizeof(irc));
    session_init_irc(session);
    irc.session = session;        

    start_loopers(&irc);

    while (1) 
    {
        irc.response[0] = 0;
        irc.from[0] = 0;
        memset(&irc.message, 0, sizeof (irc.message));

        if (socket_read_line(&session->socket, line) < 0)
            break;

        irc_process_line(&irc, line);

        if (irc.response[0])
            socket_send_message(&session->socket, irc.response);
    }
}


void session_create(struct session_t * session)
{ 
    socket_connect(&session->socket);
}

void session_destroy(struct session_t * session)
{

}

