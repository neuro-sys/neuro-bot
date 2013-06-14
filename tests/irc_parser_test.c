#include "../irc_parser.h"

#include <stdio.h>
#include <string.h>

char *irc_lines[] = {
    ":kokobot!~ircbot@88.240.45.200 JOIN #gameover",
    ":leguin.freenode.net 353 kokobot = #gameover :kokobot @ChanServ",
    ":leguin.freenode.net 366 kokobot #gameover :End of /NAMES list",
    ":ChanServ!ChanServ@services. NOTICE kokobot :[#gameover] sikerim",
    ":fooobar KOMUT param1 param2 :test",
    "PING :foo.bar",
    NULL
};

int main(int argc, char *argv)
{
    struct message_t message;
    int i;

    memset(&message, 0, sizeof (message));

    for (i = 0; irc_lines[i] != NULL; i++) {
        printf("[RAW LINE: %s]\n", irc_lines[i]);
        memset(&message, 0, sizeof (message));
        irc_parser(&message, irc_lines[i]);
        print_message_t(&message);
    }

    return 0;
}
