#include "../irc.h"

#include <stdio.h>
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

        len = strcspn(src, " ");
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

/* 
 * message    =  [ ":" prefix SPACE ] command [ params ] crlf 
 *
 */
static void irc_parser(struct message_t * message, const char * line)
{
    char buffer[510];
    size_t n;

    if (line == NULL)
        return;

    /* If starts with ':', then has prefix. */
    if (line[0] == ':') {
        n = strcspn(line, " ");
        strncpy(buffer, line, n);
        buffer[n] = '\0';
        parse_prefix(&message->prefix, buffer+1);
        line += n + 1;
    } 

    /* Then get the command. */
    n = strcspn(line, " ");
    strncpy(message->command, line, n);
    message->command[n] = '\0';
    line += n+1;

    /* Get params if there's. */
    if ( (n = strcspn(line, " \r\n")) ) {
        int i = 0;
        char params[200];
        char * t;

        strcpy(params, line);
        /* Get the trailing if there is. */
        if ( (t = strchr(params, ':')) ) { 
            strcpy(message->trailing, t+1);
            *t = '\0';
        }
        t = strtok(params, " \r\n");
        strcpy(message->params.list[i++], t);
        while ( (t = strtok(NULL, " ")) && i <= 14) {
            strcpy(message->params.list[i++], t);
        }
        message->params.list[i][0] = '\0';
    }
}


void print_message_t(struct message_t * message)
{
    if (message->prefix.servername[0])
        printf("Serv: (%s) ", message->prefix.servername);
    else
        printf("Nick: (%s!%s@%s) ", message->prefix.nickname.nickname
                                , message->prefix.nickname.user
                                , message->prefix.nickname.host);

    if (message->command[0])
        printf("(%s) ", message->command);

    if (message->params.list[0][0]) {
        int i = 0;
        char * it;

        printf("{");
        while ( message->params.list[i][0] ) {
            printf("\"%s\"", message->params.list[i++]);
            if (message->params.list[i][0])
                printf(", ");
        }
        printf("}");
    }
    if (message->trailing[0])
        printf(" => %s", message->trailing);

    printf("\n");
}

char *irc_lines[] = {
    ":kokobot!~ircbot@88.240.45.200 JOIN #gameover",
    ":leguin.freenode.net 353 kokobot = #gameover :kokobot @ChanServ",
    ":leguin.freenode.net 366 kokobot #gameover :End of /NAMES list",
    ":ChanServ!ChanServ@services. NOTICE kokobot :[#gameover] sikerim",
    ":fooobar KOMUT param1 param2 :test",
    NULL
};

int main(int argc, char *argv)
{
    struct message_t message;
    int i;

    memset(&message, 0, sizeof (message));

    for (i = 0; irc_lines[i] != NULL; i++) {
//        printf("RAW LINE: %s\n", irc_lines[i]);
        memset(&message, 0, sizeof (message));
        irc_parser(&message, irc_lines[i]);
        print_message_t(&message);
    }

    return 0;
}
