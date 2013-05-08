#ifndef __IRC_CMD_H
#define __IRC_CMD_H

extern void irc_set_nick(char * nickname, char * buffer);
extern void irc_set_user(char * user, char * host, char * buffer);
extern void irc_identify_to_auth(char * password, char * buffer);
extern void irc_join_channel(char * channel, char * buffer);

#endif

