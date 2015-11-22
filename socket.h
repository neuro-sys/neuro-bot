#ifndef __SOCKET_H_
#define __SOCKET_H_

#define O_RDONLY	0x0000
#define O_WRONLY	0x0001
#define O_RDWR		0x0002
#define O_ACCMODE	0x0003

int socket_open (const char*, int);
int socket_read (int, void*, unsigned int);
int socket_connect(char * host_name, int port);
int socket_readline(int sockfd, char * buf, int buf_len);
int socket_write(int sockfd, char * buf, int buf_len);
int socket_close(int sockfd);

#endif // __SOCKET_H_
