#include "irc_cmd.h"
#include "irc.h"

#include <stdio.h>

void irc_set_nick(char * nickname, char * buffer)
{
    snprintf(buffer, MAX_IRC_MSG, "NICK %s\r\n", nickname); 
}

void irc_set_user(char * user, char * host, char * buffer)
{
    snprintf(buffer, MAX_IRC_MSG, "USER %s 8 * :%s\r\n\r\n",
            user, host); 
}

void irc_identify_to_auth(char * password, char * buffer)
{
    sprintf(buffer, "PRIVMSG NickServ :identify %s\r\n", password);
}

void irc_join_channel(char * channel, char * buffer)
{
    sprintf(buffer, "JOIN %s\r\n", channel);
}
