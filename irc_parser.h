#ifndef __IRC_PARSER_H
#define __IRC_PARSER_H

/** 
 *( nickname [ [ "!" user ] "@" host ] )
 *
 */
struct nickname_t {
    char nickname[100];
    char user[100];
    char host[100];
};

/**
 * prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
 */
struct prefix_t {
    char servername[100];
    struct nickname_t nickname;
};

struct params_t {
    char list[14][50];
};

struct message_t {
    struct prefix_t prefix;
    char command[10];
    struct params_t params;
    char trailing[500];
};

void print_message_t(struct message_t * message);

/** 
 * Parse the string pointed to by line in the following format into 
 * a previously allocated struct message_t pointed to by message.
 *
 * message    =  [ ":" prefix SPACE ] command [ params ] crlf 
 *
 */
void irc_parser(struct message_t * message, const char * line);

#endif

