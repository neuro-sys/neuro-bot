#include "irc_parser.h"

#include <stdio.h>
#include <string.h>

static void parse_prefix_nickname(struct nickname_t * nickname, const char * src)
{
    char * c;
    size_t n;

    if (src == NULL)
        return;

    if ((c = strchr(src, '@'))) {
        char * k;
        size_t len;

        len = strlen(src);
        strncpy(nickname->host, c+1, len);
        nickname->host[len] = '\0';
        if ((k = strchr(src, '!'))) {
            strncpy(nickname->user, k+1, (c - k - 1));
            nickname->user[c - k - 1] = '\0';
        }
    } 
    n = strcspn(src, "!@ ");
    strncpy(nickname->nickname, src, n);
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
        strcpy(prefix->servername, src);
    else
        parse_prefix_nickname(&prefix->nickname, src);
}

static void parse_params(char params[][50], char * trailing, const char * src)
{
    int i = 0;        /* They are: `param1 param2 ... [:Trailing ...]CRLF'  */
    char buf[510];
    char * t;

    strcpy(buf, src); /* Copy the line in a buffer to tokenize it. */
    /* Get the `trailing' first, which marks the ending of params, if there is. */
    if ((t = strstr(buf, " :"))) { /* It starts with a SPACE preceding a ':'. */ 
        char * end = strchr(t, '\r');
        *end = '\0';
        strcpy(trailing, t+2); /* Skip " :" character by two. */
        *t = '\0'; /* Put a NUL terminator where the trailing begins, and params end.*/
    }
    t = strtok(buf, " \r\n");
    if (t) { /* Are there any params? */
        strcpy(params[i++], t);
        while ( (t = strtok(NULL, " \r\n")) && i <= 14) {
            strcpy(params[i++], t);
        }
        params[i][0] = '\0';
    }
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
    if (strlen(line))  /* If there's any chars left in the string so far,    */
        parse_params(message->params, message->trailing, line);
}

void print_message_t(struct message_t * message)
{
    if (message->prefix.servername[0])
        fprintf(stderr, "Serv: (%s) ", message->prefix.servername);
    else if (message->prefix.nickname.nickname[0]) {
        fprintf(stderr, "Nick: (%s", message->prefix.nickname.nickname);
        if (message->prefix.nickname.user[0])
            fprintf(stderr, "!%s", message->prefix.nickname.user);
        if (message->prefix.nickname.host[0])
            fprintf(stderr, "@%s ", message->prefix.nickname.host);
        fprintf(stderr, ") ");
    }

    if (message->command[0])
        fprintf(stderr, "(%s) ", message->command);

    if (message->params[0][0]) {
        int i = 0;

        fprintf(stderr, "{");
        while (message->params[i][0]) {
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

#ifdef TEST_IRC_PARSER
int main(int argc, char *argv)
{
    struct message_t message;
    int i;

    char *irc_lines[] = {
        ":kokobot!~ircbot@88.240.45.200 JOIN #gameover\r\n",
        ":leguin.freenode.net 353 kokobot = #gameover :kokobot @ChanServ\r\n",
        ":leguin.freenode.net 366 kokobot #gameover :End of /NAMES list\r\n",
        ":ChanServ!ChanServ@services. NOTICE kokobot :[#gameover] sikerim\r\n",
        ":fooobar KOMUT param1 param2 :test\r\n",
        "PING :foo.bar\r\n",
        ":ChanServ!ChanServ@services. MODE #botwar +o KOKBOT\r\n",
        NULL
    };

    memset(&message, 0, sizeof (message));

    for (i = 0; irc_lines[i] != NULL; i++) {
        memset(&message, 0, sizeof (message));
        irc_parser(&message, irc_lines[i]);
        print_message_t(&message);
    }

    return 0;
}
#endif

