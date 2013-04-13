#ifndef __NEUROBOTAPI_H
#define __NEUROBOTAPI_H

#define MAX_IRC_MSG 510

/* Helper functions */
char *  (*curl_perform)(char * url);
void    (*n_strip_tags)(char * dest, char * src);
char *  (*n_get_tag_value)(char * body, char * tagname);

#define MAX_KEYWORDS 10

char * keywords[MAX_KEYWORDS];


struct network_t {
    char        * host_name;
    int         port;
    int         sockfd;
};

struct session_t {
    void                (* run) (struct session_t * session);
    struct network_t    network;
    char                * nickname;
    char                * password;
    char                * admin;
};

struct srv_message_t {
    char prefix[100];
    char command[100];
    char params[100];
};

struct irc_t {
    struct srv_message_t srv_msg;
    char                 request[MAX_IRC_MSG];
    char                 response[MAX_IRC_MSG];
    char                 nick_to_msg[100];
    char                 from[100];
    struct session_t     * session;
};

#endif
