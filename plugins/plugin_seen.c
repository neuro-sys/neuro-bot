#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

char * keywords[100] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","0","1","2","3","4","5","6","7","8","9"};

struct plugin_t * plugin;

#define MAX_SEEN_STRING_SIZE 200

struct seen_t {
    char lastmsg[MAX_SEEN_STRING_SIZE];
    char lasttime[MAX_SEEN_STRING_SIZE];
};

/* bad hack, just dump single quotes */
static char * escape_quotes(char * str)
{
    for (; *str != 0; str++) {
        if (*str == '\'') *str = '\"';
    } 

    return str;
}

/* @TODO: later to add: check if already opened, if not, re-open the database (might have got closed somehow) */
static struct sqlite3 * get_db_connection(void)
{
    int sqliteStatus;
    static sqlite3 *seendb;

    sqliteStatus = sqlite3_open_v2("plugins/seen.db", &seendb, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    if (sqliteStatus != SQLITE_OK) {
        debug("wasn't able to open/create database");
    }

    return seendb;
}

static void close_db_connection(struct sqlite3 * seendb)
{
    sqlite3_close(seendb);
}

int db_does_name_exist(void * param, int argc, char ** argv, char ** column_names)
{
    *((int *) param) = 1;

    return 0;
}

int db_read_seen(void * param, int argc, char ** argv, char ** column_names)
{
    struct seen_t * seen = (struct seen_t *) param;
    int i;

    for (i = 0; i < argc; i++) {
        char * col_name = column_names[i];

        if (strcmp(col_name, "lastmsg") == 0) {
            snprintf(seen->lastmsg, MAX_SEEN_STRING_SIZE, "%s", argv[i]);
        }

        if (strcmp(col_name, "lasttime") == 0) {
            snprintf(seen->lasttime, MAX_SEEN_STRING_SIZE, "%s", argv[i]);
        }
    }

    return 0;
}

static const char * get_command_parameter(char * trailing)
{
    char * param = strchr(trailing, ' ');
    if (param == NULL || *++param == 0) {
        return NULL;
    }

    return param;
}

void handle_command(void)
{
    char *zErrMsg = 0;
    int sqliteStatus;
    struct seen_t seen;
    char sql[512];
    struct sqlite3 * seendb = get_db_connection();
    char response[512];
    const char * who;

    memset(&seen, 0, sizeof(seen));

    who = get_command_parameter(plugin->irc->message.trailing);

    if (who == NULL || strspn(who, " \t\r\n") > 0) {
        snprintf(response, 512, "PRIVMSG %s :Seen who?",
            plugin->irc->from
        );
        plugin->send_message(plugin->irc, response);
        return;
    }

    if (strcmp(plugin->irc->session->nickname, who) == 0) {
        snprintf(response, 512, "PRIVMSG %s :I'm here.",
            plugin->irc->from
        );
        plugin->send_message(plugin->irc, response);
        return;
    }

    sprintf(
        sql,
        "select * from seen where name = '%s';",
        who
    );
    sqliteStatus = sqlite3_exec(seendb, sql, db_read_seen, &seen, &zErrMsg);

    if (strcmp(seen.lasttime, "") == 0) {
        snprintf(response, 512, "PRIVMSG %s :I haven't seen %s around.",
            plugin->irc->from,
            who
        );
        plugin->send_message(plugin->irc, response);
        return;
    }

    snprintf(response, 512, "PRIVMSG %s :%s is last seen saying \"%s\" at %s.",
        plugin->irc->from,
        who,
        seen.lastmsg,
        seen.lasttime
    );
    plugin->send_message(plugin->irc, response);
}

void handle_grep(void)
{
    char *zErrMsg = 0;
    int sqliteStatus;
    int exists = 0;
    char sql[512];
    struct sqlite3 * seendb = get_db_connection();

    sprintf(
        sql,
        "SELECT * FROM SEEN WHERE NAME = '%s';",
        plugin->irc->message.prefix.nickname.nickname
    );
    sqliteStatus = sqlite3_exec(seendb,sql,db_does_name_exist, &exists , &zErrMsg);

    if (exists) {
        sprintf(
                sql,
                "UPDATE SEEN SET LASTMSG = '%s', LASTTIME = CURRENT_TIMESTAMP WHERE NAME = '%s';",
                plugin->irc->message.trailing,
                plugin->irc->message.prefix.nickname.nickname
               );
        sqliteStatus = sqlite3_exec(seendb,sql,0,0,NULL);
    } else {
        sprintf(
                sql,
                "INSERT INTO SEEN(NAME,LASTMSG,LASTTIME) VALUES('%s','%s', CURRENT_TIMESTAMP);",
                plugin->irc->message.prefix.nickname.nickname,
                plugin->irc->message.trailing
               );
        sqliteStatus = sqlite3_exec(seendb,sql,0,0,NULL);
    }

    close_db_connection(seendb);
}

void run(void)
{
    escape_quotes(plugin->irc->message.trailing);
    if (IS_BIT_ON(plugin->is_command, 2)) {
        handle_command();
    } else if (IS_BIT_ON(plugin->is_grep, 2)) {
        handle_grep();
    }
}


static void create_tables(void) 
{
    char *zErrMsg = 0;
    int sqliteStatus;
    struct sqlite3 * seendb = get_db_connection();
    char * create_script = "CREATE TABLE IF NOT EXISTS SEEN (NAME, LASTMSG, LASTTIME);";

    sqliteStatus = sqlite3_exec(seendb,create_script,0,0,&zErrMsg);
    if (sqliteStatus != SQLITE_OK) {
        abort();
    }


    close_db_connection(seendb);
}



struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "seen";
    plugin->is_looper  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 1;

    create_tables();

    return plugin;

}
