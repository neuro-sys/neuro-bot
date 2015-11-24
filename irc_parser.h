#ifndef __IRC_PARSER_H
#define __IRC_PARSER_H

#define MAX_IRC_MSG    510
#define MAX_PARAMS      14

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

struct message_t {
    struct prefix_t prefix;
    char command[10];
    char params[MAX_PARAMS][50];
    char trailing[MAX_IRC_MSG];
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

