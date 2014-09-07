
#include "plugin_client.h"

#include "utils/curl_wrapper.h"
#include "utils/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct plugin_t * plugin;

struct youtube_t {
	char title[512];
	char url[512];
};

static struct youtube_t * parse_json_youtube(char * data)
{
	json_value * root, * entry;
	struct youtube_t * youtube;

	youtube = malloc(sizeof (struct youtube_t));

	root = json_parse(data);

	if (root == NULL)
		return NULL;

	entry = n_json_find_object(root, "entry");

	if (entry == NULL)
		return NULL;


	if (entry->u.array.length == 0) {
		return NULL;
	}

      
	json_value * val;
        
	val = entry->u.array.values[0];


	json_value * title, * link;

	title = n_json_find_object(n_json_find_object(val, "title"), "$t");
	strcpy(youtube->title, title->u.string.ptr);

	link = n_json_find_object(n_json_find_object(val, "link")->u.array.values[0], "href");
	strcpy(youtube->url, link->u.string.ptr);
	*strchr(youtube->url, '&') = '\0';


	json_value_free(root);

	return youtube;
}

static void string_replace_all(char * str, char this, char that)
{
	char * ptr;

	ptr = str;

	while (*ptr != '\0') {
		if (*ptr == this) *ptr = that;
		ptr++;
	}
}

static void parse_search_keyword(struct irc_t * irc, char * dest, size_t dest_len)
{
	char request[512];
	char * t;

	strcpy(request, irc->message.trailing);

	t = strchr(request, ' ');

	if (t == NULL) {
		return;
	}

	t++;

	string_replace_all(t, ' ', '+');

	strncpy(dest, t, dest_len);
}

static char * request_youtube_api(char * keyword)
{
	struct http_req * http;
	char url[1024];
	char * response;

	sprintf(url, "http://gdata.youtube.com/feeds/api/videos?alt=json&ver=2&orderBy=relevance&q=%s&max-results=1", keyword);

	http = curl_perform(url, NULL);

	response = strdup(http->body);

	free(http->header);
	free(http->body);
	free(http);

	return response;
}

void run(void)
{
	char response[512];
	char search_keyword[400];
	char * json_data;
	struct youtube_t * youtube;

	search_keyword[0] = '\0';

	parse_search_keyword(plugin->irc, search_keyword, 400);

	if (search_keyword[0] == '\0') {
		return;
	}

	json_data = request_youtube_api(search_keyword);

	youtube = parse_json_youtube(json_data);

	free(json_data);
    
	if (youtube == NULL)
		return;

	sprintf(response, "PRIVMSG %s :%s - %s", plugin->irc->from, youtube->title, youtube->url);

	free(youtube);
#ifndef TEST_PLUGIN_YT
	plugin->send_message(plugin->irc, response);
#else
	fprintf(stdout, "%s\n", response);
#endif
}

 struct plugin_t * init(void)
 {
	plugin = malloc(sizeof (struct plugin_t));
	memset(plugin, 0, sizeof *plugin);

	plugin->run        = run;
	plugin->name       = "yt";
	plugin->is_looper  = 0;
	plugin->is_command = 1;
	plugin->is_grep    = 0;

	return plugin;
}

#ifdef TEST_PLUGIN_YT
 int main(int argc, char *argv[])
 {
	struct plugin_t p;
	struct irc_t irc;

	plugin = &p;
	strcpy(irc.message.trailing, ".yt coil triple sun");
	plugin->irc = &irc;

	run();

	return 0;
}

#endif
