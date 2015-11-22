#ifndef __DL_H_
#define __DL_H_

void * dl_open(char * dl);
void * dl_sym(void * library, char * sym);

#endif // __DL_H_
