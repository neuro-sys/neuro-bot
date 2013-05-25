#include "neurobotapi.h"

void     (*network_send_message) (struct network_t * network, char * message);
#ifdef _WIN32
__declspec(dllexport)
#endif
void init(char * ** keyword_ptr, void (*network) (struct network_t * n, char *m), int * looper_ptr)
{
    *keyword_ptr = keywords;
    *looper_ptr  = looper;
    network_send_message = network;
}


