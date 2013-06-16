#ifndef __SOCKET_H_
#define __SOCKET_H_

#if defined (_WIN32)
  #include <ws2tcpip.h>
  #define t_connect t_connect_win
  extern int t_connect_win(char *host, char *port);
#elif defined (unix)
  #define t_connect t_connect_unix
  extern int t_connect_unix(char *host, char *port);
#endif

struct socket_t {
    char        * host_name;
    char        * port;
    int         sockfd;
};

extern void     socket_connect         (struct socket_t * socket);
extern int      socket_read_line       (struct socket_t * socket, char * buf);
extern void     socket_send_message    (struct socket_t * socket, char * message);

#endif
