#include "session.h"
#include "network.h"
#include "irc.h"
#include "global.h"
#include "irc_cmd.h"
#include "module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

pthread_t threads[10];

static void session_init_irc(struct session_t * session)
{
    char message[MAX_IRC_MSG];

    irc_set_nick(session->nickname, message);
    network_send_message(&session->network, message);

    irc_set_user("ircbot", "github.com/neuro-sys/neuro-bot", message);
    network_send_message(&session->network, message);

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

    loopers = module_get_loopers();
    if (!loopers)
        return;
    
    iterator = loopers;
    while (*iterator) {
       struct mod_c_t * mod;
       struct thread_struct * ts;

       ts = malloc(sizeof (struct thread_struct));

       mod = *iterator++;

       ts->mod = mod;
       ts->irc = irc;

       fprintf(stderr, "Looper %s is started\n", mod->mod_name);
       pthread_create(&threads[0], NULL, looper_thread, ts);
    }

    free(loopers);
}

void session_run(struct session_t * session)
{ 
    char          line[MAX_IRC_MSG];
    struct irc_t  irc;

    memset(&irc, 0, sizeof(struct irc_t));
    session_init_irc(session);
    irc.session = session;        

    start_loopers(&irc);

    while (1) 
    {
        irc.response[0] = '\0';

        if (network_read_line(&session->network, line) < 0)
            break;

        irc_process_line(&irc, line);

        if (irc.response[0])
            network_send_message(&session->network, irc.response);
    }
}


void session_create(struct session_t * session)
{ 
    network_connect(&session->network);
}

void session_destroy(struct session_t * session)
{

}

