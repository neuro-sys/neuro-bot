#include "plugin_client.h"

#include "utils/curl_wrapper.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include <sys/queue.h>


const char * xp_channel_title = "/rss/channel/title";
const char * xp_channel_link = "/rss/channel/link";
const char * xp_channel_description = "/rss/channel/description";

const char * xp_channel_items = "/rss/channel/item";

struct plugin_t * plugin;

struct rss_entity_s {
    char * code;
    char * title;
    char * rss_url;
    char * url;
    char * description;
    char * updated;
        
    LIST_ENTRY(rss_entity_s) rss_entity_list;
};

static struct sqlite3 * get_db_connection(void)
{
    int sqliteStatus;
    static sqlite3 * db;

    sqliteStatus = sqlite3_open_v2("plugins/rss.db", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    if (sqliteStatus != SQLITE_OK) {
        debug("wasn't able to open/create database");
    }

    return db;
}

static void close_db_connection(struct sqlite3 * db)
{
    sqlite3_close(db);
}

int db_cb_create_rss_entity(void * param, int argc, char ** argv, char ** column_names)
{
    LIST_HEAD(rss_list_head, rss_entity_s) * rss_list_head = param;
    int i;
    struct rss_entity_s * rss = malloc(sizeof (struct rss_entity_s));
    memset(rss, 0, sizeof(struct rss_entity_s));

    for (i = 0; i < argc; i++) {
        char * col_name = column_names[i];

        if (strcmp(col_name, "CODE") == 0) {
            rss->code = strdup(argv[i]);
        }
	
        if (strcmp(col_name, "RSS_URL") == 0) {
            rss->rss_url = strdup(argv[i]);
        }

        if (strcmp(col_name, "TITLE") == 0) {
            rss->title = strdup(argv[i]);
        }
    
        if (strcmp(col_name, "URL") == 0) {
            rss->url = strdup(argv[i]);
        }

        if (strcmp(col_name, "UPDATED") == 0) {
            rss->updated = strdup(argv[i]);
        }
    }

    LIST_INSERT_HEAD(rss_list_head, rss, rss_entity_list);

    return 0;
}

void rss_entity_insert(struct rss_entity_s * entity, char * code, char * rss_url)
{
    char *zErrMsg = 0;
    int sqliteStatus;
    struct sqlite3 * db = get_db_connection();
    char statement[1024];
    LIST_HEAD(rss_list_head, rss_entity_s) rss_list_head; 
    LIST_INIT(&rss_list_head);

    sprintf(statement, "SELECT * FROM RSS WHERE RSS_URL = '%s'", rss_url);

    sqliteStatus = sqlite3_exec(db, statement, db_cb_create_rss_entity, &rss_list_head, &zErrMsg);
    if (sqliteStatus != SQLITE_OK) {
        char response[MAX_IRC_MSG];

        sprintf(response, "PRIVMSG %s :Could not query database.", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        struct rss_entity_s * iterator;
        LIST_FOREACH(iterator, &rss_list_head, rss_entity_list) free(iterator);
        return;
    }

    if (rss_list_head.lh_first != NULL && rss_list_head.lh_first->code != NULL) {
        char response[MAX_IRC_MSG];

        sprintf(response, "PRIVMSG %s :'%s' already exists with the code '%s'", plugin->irc->from, rss_url, rss_list_head.lh_first->code);
        plugin->send_message(plugin->irc, response);
        return;
    }

    struct rss_entity_s * iterator;
    LIST_FOREACH(iterator, &rss_list_head, rss_entity_list) free(iterator);

    sprintf(statement, "INSERT INTO RSS (CODE, RSS_URL, TITLE, URL) VALUES( '%s', '%s', '%s', '%s');", code, rss_url, entity->title, entity->url);

    sqliteStatus = sqlite3_exec(db, statement, 0, 0, &zErrMsg);
    if (sqliteStatus != SQLITE_OK) {
        char response[MAX_IRC_MSG];

        sprintf(response, "PRIVMSG %s :Could not insert '%s' into database.", plugin->irc->from, code);
        plugin->send_message(plugin->irc, response);
        return;
    }

    char response[MAX_IRC_MSG];

    sprintf(response, "PRIVMSG %s :'%s' successfully inserted. %s - %s - %s", plugin->irc->from, code, entity->title, entity->url, entity->description);
    plugin->send_message(plugin->irc, response);

    close_db_connection(db);
}

struct rss_entity_s * rss_parse_feed(struct rss_entity_s * rss_entity, xmlDocPtr doc)
{
    memset(rss_entity, 0, sizeof(struct rss_entity_s));
    
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;

    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        debug("Unable to create XPath context\n");
        return NULL;
    }

    xpathObj = xmlXPathEvalExpression((xmlChar *) xp_channel_title, xpathCtx);
    if (xpathObj == NULL) {
        debug("Unable to evaluate XPath expression.\n");
        return NULL;
    }
    rss_entity->title = (char *) xmlNodeGetContent(xpathObj->nodesetval->nodeTab[0]);

    xpathObj = xmlXPathEvalExpression((xmlChar *) xp_channel_link, xpathCtx);
    if (xpathObj == NULL) {
        debug("Unable to evaluate XPath expression.\n");
        return NULL;
    }
    rss_entity->url = (char *) xmlNodeGetContent(xpathObj->nodesetval->nodeTab[0]);


    xpathObj = xmlXPathEvalExpression((xmlChar *) xp_channel_description, xpathCtx);
    if (xpathObj == NULL) {
        debug("Unable to evaluate XPath expression.\n");
        return NULL;
    }
    rss_entity->description = (char *) xmlNodeGetContent(xpathObj->nodesetval->nodeTab[0]);
   
    return rss_entity;
}

void rss_del(char * code)
{
    char *zErrMsg = 0;
    int sqliteStatus;
    struct sqlite3 * db = get_db_connection();
    char statement[250];

    sprintf(statement, "DELETE FROM RSS WHERE CODE = '%s';", code);

    sqliteStatus = sqlite3_exec(db, statement, NULL, NULL, &zErrMsg);
    if (sqliteStatus != SQLITE_OK) {
        char response[MAX_IRC_MSG];

        sprintf(response, "PRIVMSG %s :Could not delete '%s'.", plugin->irc->from, code);
        plugin->send_message(plugin->irc, response);
        return;
    }

    char response[MAX_IRC_MSG];

    sprintf(response, "PRIVMSG %s :'%s' has been deleted.", plugin->irc->from, code);
    plugin->send_message(plugin->irc, response);
}

void rss_add(char * code, char * url)
{
    struct http_req * http = NULL;
    struct rss_entity_s rss_entity;

    xmlDocPtr doc;

    http = curl_perform(url, NULL);

    if (http == NULL || http->body == NULL)
        return;

#ifdef TEST_PLUGIN_RSS
    debug("%s\n", http->body);
#endif

    doc = xmlReadMemory(http->body, strlen(http->body), NULL, NULL, 0);
    if (doc == NULL) {
        char response[MAX_IRC_MSG];

        sprintf(response, "PRIVMSG %s :RSS %s is not valid.", plugin->irc->from, url);
        plugin->send_message(plugin->irc, response);
        return;
    }

    if (rss_parse_feed(&rss_entity, doc) == NULL) {
        goto CLEAN;
    }

    rss_entity_insert(&rss_entity, code, url);

CLEAN:
    free(http->body);
    free(http->header);
    free(http);

    xmlFreeDoc(doc);
    xmlCleanupParser();
}

void rss_list(void)
{
    char *zErrMsg = 0;
    int sqliteStatus;
    struct sqlite3 * db = get_db_connection();
    char statement[250];
    struct rss_entity_s * iterator;

    LIST_HEAD(rss_list_head, rss_entity_s) rss_list_head; 
    LIST_INIT(&rss_list_head);

    sprintf(statement, "SELECT R.CODE, R.RSS_URL, R.TITLE, R.URL, R.UPDATED FROM RSS R;");

    sqliteStatus = sqlite3_exec(db, statement, db_cb_create_rss_entity, &rss_list_head, &zErrMsg);
    if (sqliteStatus != SQLITE_OK) {
        debug("error");
        return;
    }

    if (rss_list_head.lh_first == NULL) {
        char response[MAX_IRC_MSG];

        snprintf(response, MAX_IRC_MSG, "PRIVMSG %s :No RSS record found.", plugin->irc->from);
        plugin->send_message(plugin->irc, response);

        return;
    }

    LIST_FOREACH(iterator, &rss_list_head, rss_entity_list) {
        struct rss_entity_s * rss = iterator;
        char response[MAX_IRC_MSG];
       
        snprintf(response, MAX_IRC_MSG, "PRIVMSG %s :* [%s] - %s - %s - %s - %s",
            plugin->irc->from,
            rss->code,
            rss->rss_url,
            rss->title,
            rss->url,
            rss->updated
        );
        plugin->send_message(plugin->irc, response);
        free(rss);
    }

}

void rss_show(void)
{

}

void invalid_selection(void)
{
    char response[512];

    sprintf(response, "PRIVMSG %s :.rss add|list|show", plugin->irc->from);
    plugin->send_message(plugin->irc, response);
}

void decide_flow(char * trailing)
{
    struct argv_s * param;
    char * cmd;

    param = argv_parse(trailing);
    if (param == NULL) {
        invalid_selection();
        return;
    }

    if (param->argc < 2) {
        invalid_selection();
        return;
    }

    cmd = param->argv[1];

    if (strcmp(cmd, "list") == 0) {
        rss_list();
        return;
    } else if (strcmp(cmd, "add") == 0) {
        rss_add(param->argv[2], param->argv[3]); 
        return;
    } else if (strcmp(cmd, "del") == 0) {
        rss_del(param->argv[2]);
        return;
    } else if (strcmp(cmd, "show") == 0) {
        return;
    }

    invalid_selection();
    argv_free(param);
}

void run(void)
{
    decide_flow(plugin->irc->message.trailing);
}

static void create_tables(void) 
{
    char *zErrMsg = 0;
    int sqliteStatus;
    struct sqlite3 * db = get_db_connection();
    char * create_script = "CREATE TABLE IF NOT EXISTS RSS (CODE, RSS_URL PRIMARY KEY, TITLE, URL, UPDATED DEFAULT CURRENT_TIMESTAMP);";

    sqliteStatus = sqlite3_exec(db,create_script,0,0,&zErrMsg);
    if (sqliteStatus != SQLITE_OK) {
        debug("Failed to create tables\n");
        abort();
    }

    close_db_connection(db);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "rss";
    plugin->is_daemon  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    create_tables();

    return plugin;
}

#ifdef TEST_PLUGIN_RSS

void send_message(struct irc_t * irc, char * message) {
    debug("%s\n", message);
}

int main(int argc, char *argv[])
{
    struct irc_t irc;
    
    init();

    memset(&irc, 0, sizeof(irc));

    plugin->irc = &irc;
    plugin->send_message = &send_message;

    sprintf(irc.message.trailing, ".rss add hackernews https://news.ycombinator.com/rss");

    run();
    return 0;
}
#endif

