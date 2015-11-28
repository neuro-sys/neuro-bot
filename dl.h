#ifndef __DL_H_
#define __DL_H_

typedef void * dl_library_t;

dl_library_t dl_open(char * dl);
void dl_close(dl_library_t library);
void * dl_sym(dl_library_t library, char * sym);

#endif // __DL_H_
