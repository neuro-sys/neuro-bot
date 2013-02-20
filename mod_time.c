#include "irc.h"
#include "global.h"

#include <stdio.h>
#include <time.h>

void mod_cmd_time(struct irc_t * irc)
{
  time_t now;
    
  time(&now);
  sprintf(irc->response, "PRIVMSG %s :%s\r\n", irc->from, ctime(&now));
}
