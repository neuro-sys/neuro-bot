#include "session.h"

#include "socket.h"
#include "irc.h"
#include "plugin.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

void prepare_plugins(struct irc_t * irc)
{
    struct plugin_list_t * it;

    debug("Session preparing plugins before connecting to the server.\n");
    for (it = plugin_list_head; it != NULL; it = it->next) {
        debug("Name: [%s]\n", it->cur->name);
        it->cur->irc = irc;
    }
}

/**
 * TODO: Use a dynamic structure to keep track of threads.
 */
static pthread_t plugin_threads[10];

static void session_init_irc(struct session_t * session)
{
    char message[MAX_IRC_MSG];

    irc_set_nick(session->nickname, message);
    socket_send_message(&session->socket, message);

    irc_set_user("ircbot", "github.com/neuro-sys/neuro-bot", message);
    socket_send_message(&session->socket, message);
}

static void *start_thread(void * pdata)
{
    ((struct plugin_t *)pdata)->run();

    return NULL;
}

static void start_loopers(struct irc_t * irc)
{

    struct plugin_list_t * it;
    int i, err;

    for (i = 0, it = plugin_list_head; it != NULL; it = it->next, i++) {
        if (!it->cur->is_looper || it->cur->is_manager)
            continue;

        if (i == 10) {
            debug("Passed the maximum number of loopers limit.\n");
            break;
        }

        debug("Looper plugin [%s] is started\n", it->cur->name);
        if ((err = pthread_create(&plugin_threads[i], NULL, start_thread, it->cur)) != 0) {
            debug("Thread could not be started. pthread_create, errno = %d\n", err);
        }
    }
}

void session_run(struct session_t * session)
{ 
    char          line[MAX_IRC_MSG];
    struct irc_t  irc;

    memset(&irc, 0, sizeof(irc));
    irc.session = session;        
    prepare_plugins(&irc);

    /* Conect to the server specified in socket_t struct. */
    if ( socket_connect(&irc.session->socket) < 0 ) {
        debug("Unable to connect to %s:%s\n", irc.session->socket.host_name, irc.session->socket.port);
        exit(EXIT_SUCCESS);
    }
    /* Do one time initialization work after connecting to the server. */
    session_init_irc(irc.session);

    start_loopers(&irc);

    while (1) 
    {

        irc.response[0] = 0;
        irc.from[0]     = 0;
        memset(&irc.message, 0, sizeof (irc.message));

        if (socket_read_line(&irc.session->socket, line) < 0)
            break;

        irc_process_line(&irc, line);

        if (irc.response[0])
            socket_send_message(&irc.session->socket, irc.response);
    }
}

