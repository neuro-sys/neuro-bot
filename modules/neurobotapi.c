#include "neurobotapi.h"

void     (*irc_send) (struct socket_t * socket, char * message);
#ifdef _WIN32
__declspec(dllexport)
#endif
void init(char * ** keyword_ptr, void (*socket) (struct socket_t * n, char *m), int * looper_ptr)
{
    *keyword_ptr = keywords;
    *looper_ptr  = looper;
    irc_send = socket;
}


