#ifndef __MOD_EXAMPLE_H
#define __MOD_EXAMPLE_H

#include "irc.h"

/* 
  Implement either or both of this. Note the prefixes mod_line_* and mod_cmd_*.
  
    mod_line_* is for modules that scan a line without a COMMAND_PREFIX.

    mod_cmd_* is for modules that processes only lines that starts with
  COMMAND_PREFIX.

  The functions should fill irc->response with the appropriate message
  for the server to understand, and return nothing.

  Example:
    sprintf(irc->response, "PRIVMSG %s :Hello!\r\n", irc->from); 
*/

extern void mod_line_example(struct irc_t * irc):
extern void mod_cmd_example(strcut irc_t * irc);

#endif
