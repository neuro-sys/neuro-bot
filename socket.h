#ifndef __SOCKET_H
#define __SOCKET_H

#include "global.h"

#if defined (_WIN32)
  #include <ws2tcpip.h>
  #define t_connect t_connect_win
  extern int t_connect_win(char *host, char *port);
#elif defined (unix)
  #define t_connect t_connect_unix
  extern int t_connect_unix(char *host, char *port);
#endif


#endif
