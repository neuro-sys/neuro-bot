#include "plugin_client.h"

#include "utils/curl_wrapper.h"
#include "utils/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WEATHER_API "http://api.openweathermap.org/data/2.5/weather?q="

#define MPS_TO_KMH(A) A*3.6

struct plugin_t * plugin;

struct weather_s {
    double lon, lat, temp, pressure, humidity, wind_speed, wind_deg, clouds;
    char * main, * name;
};

static void weather_free(struct weather_s * weather)
{
    free(weather->main);
    free(weather->name);
    free(weather);
}


static struct weather_s * parse_json(char * json_payload)
{
    struct weather_s * weather;
    json_value * root, * node;

    weather = malloc(sizeof (struct weather_s));

    if (!(root = json_parse(json_payload))) {
        return NULL;
    }

    if (!(node = n_json_find_object(root, "main"))) {
        return NULL;
    }
    weather->main = strdup(node->u.string.ptr);

    if (!(node = n_json_find_object(root, "name"))) {
        return NULL;
    }
    weather->name = strdup(node->u.string.ptr);

    if (!(node = n_json_find_object(root, "temp"))) {
        return NULL;
    }
    weather->temp = node->u.dbl;

    if (!(node = n_json_find_object(root, "speed"))) {
        return NULL;
    }
    weather->wind_speed= node->u.dbl;

    if (!(node = n_json_find_object(root, "all"))) {
        return NULL;
    }
    weather->clouds = node->u.dbl;

    json_value_free(root);

    return weather;
}

static char * get_json_weather_data(const char * city_name)
{
    char url[512];
    char * ret;
    struct http_req * get; 

    snprintf(url, 512, "%s%s", WEATHER_API, city_name);

    get = curl_perform(url, NULL); 
    ret = strdup(get->body);

    free(get->header);
    free(get->body);
    free(get);

    return ret;
}

static const char * get_city_name(char * trailing)
{
    char * city = strchr(trailing, ' ');
    if (city == NULL || *++city == 0) {
        return NULL;
    }

    return city;
}

void run(void)
{
    char response[512];
    const char * city_name;
    char * json_payload;
    struct weather_s * weather;

    city_name = get_city_name(plugin->irc->message.trailing);
    if (city_name == NULL) {
        sprintf(response, "PRIVMSG %s :Which city?", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        return;
    }

    json_payload = get_json_weather_data(city_name);

    weather = parse_json(json_payload);
    free(json_payload);
    if (weather == NULL) {
        sprintf(response, "PRIVMSG %s :No data could have been read.!", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        return;
    }
   
    snprintf(response, 512, "PRIVMSG %s :%s (%s), Temp: %.2f \u00b0C, Wind speed: %.2f km/h, Cloud: %.f%%", 
        plugin->irc->from, weather->name, weather->main, weather->temp - 273.15, MPS_TO_KMH(weather->wind_speed), weather->clouds);
    plugin->send_message(plugin->irc, response);

    weather_free(weather);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "weather";
    plugin->is_looper  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    return plugin;
}

