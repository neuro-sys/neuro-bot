#include "irc_parser.h"

#include "irc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void parse_prefix_nickname(struct nickname_t * nickname, const char * src)
{
    char * c;
    size_t n;

    if (src == NULL)
        return;

    if ( (c = strchr(src, '@')) ) {
        char * k;
        size_t len;

        len = strlen(src);
        strncpy(nickname->host, c+1, len);
        nickname->host[len] = '\0';
        if ( (k = strchr(src, '!')) ) {
            strncpy(nickname->user, k+1, (c - k - 1));
            nickname->user[c - k - 1] = '\0';
        }
    } 
    n = strcspn(src, "!@ ");
    strncpy(nickname->nickname, src, n);
}

static void parse_prefix_servername(char * servername, const char * src)
{
    if (src == NULL)
        return;

    strcpy(servername, src);
}

/**
 * prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
 * Notes:
 * [21:26] < grawity> Server names always have a "." – nicknames never do.
 *
 */
static void parse_prefix(struct prefix_t * prefix, const char * src)
{
    if (src == NULL)
        return;

    if (!strchr(src, '@') && strchr(src, '.'))
        parse_prefix_servername(prefix->servername, src);
    else
        parse_prefix_nickname(&prefix->nickname, src);
}

/** 
 * Parses a raw IRC server message in the following format into
 * a message_t struct.
 * message    =  [ ":" prefix SPACE ] command [ params ] crlf 
 *
 */
void irc_parser(struct message_t * message, const char * line)
{
    size_t pos;

    if (line == NULL || message == NULL)
        return;

    /* If starts with ':', then has a prefix field. */
    if (line[0] == ':') {
        char buffer[510];
        pos = strcspn(line, " "); /* the prefix ends with a SPACE */
        strncpy(buffer, line, pos);
        buffer[pos] = '\0';
        parse_prefix(&message->prefix, buffer+1);
        line += pos + 1;
    } 

    /* Then get the obligatory command field. */
    pos = strcspn(line, " \r\n"); /* It ends with a SPACE or CRLF */
    strncpy(message->command, line, pos);
    message->command[pos] = '\0';
    line += pos;

    /* Get the multiple params or trailing line if there is both or either. */
    if ( strlen(line) ) { /* If there's any chars left in the string so far,    */
        int i = 0;        /* They are: `param1 param2 ... [:Trailing ...]CRLF'  */
        char params[200];
        char * t;

        strcpy(params, line); /* Copy the line in a buffer to tokenize it. */
        /* Get the `trailing' first, which marks the ending of params, if there is. */
        if ( (t = strstr(params, " :")) ) { /* It starts with a SPACE preceding a ':'. */ 
            char * end = strchr(t, '\r');
            *end = '\0';
            strcpy(message->trailing, t+2); /* Skip " :" character by two. */
            *t = '\0'; /* Put a NUL terminator where the trailing begins, and params end.*/
        }
        t = strtok(params, " \r\n");
        if (t) { /* Are there any params? */
            strcpy(message->params[i++], t);
            while ( (t = strtok(NULL, " \r\n")) && i <= 14) {
                strcpy(message->params[i++], t);
            }
            message->params[i][0] = '\0';
        }
    }
}

void print_message_t(struct message_t * message)
{
    if (message->prefix.servername[0])
        fprintf(stderr, "Serv: (%s) ", message->prefix.servername);
    else if (message->prefix.nickname.nickname[0])
        fprintf(stderr, "Nick: (%s!%s@%s) ", message->prefix.nickname.nickname
                                , message->prefix.nickname.user
                                , message->prefix.nickname.host);

    if (message->command[0])
        fprintf(stderr, "(%s) ", message->command);

    if (message->params[0][0]) {
        int i = 0;

        fprintf(stderr, "{");
        while ( message->params[i][0] ) {
            fprintf(stderr, "\"%s\"", message->params[i++]);
            if (message->params[i][0])
                fprintf(stderr, ", ");
        }
        fprintf(stderr, "}");
    }
    if (message->trailing[0])
        fprintf(stderr, " => %s", message->trailing);

    fprintf(stderr, "\n");
}
