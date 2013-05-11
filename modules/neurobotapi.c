#include "neurobotapi.h"

#ifdef _WIN32
__declspec(dllexport)
#endif
void init(char * ** keyword_ptr)
{
    *keyword_ptr = keywords;
}


