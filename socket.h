#ifndef __SOCKET_H_
#define __SOCKET_H_

#define O_RDONLY	0x0000
#define O_WRONLY	0x0001
#define O_RDWR		0x0002
#define O_ACCMODE	0x0003

typedef void * socket_t;

//int socket_open (const char*, int);
//int socket_read (int, void*, unsigned int);
socket_t socket_connect(char * host_name, int port);
int socket_readline(socket_t sock, char * buf, int buf_len);
int socket_write(socket_t sock, char * buf, int buf_len);
int socket_close(socket_t sock);

#endif // __SOCKET_H_
