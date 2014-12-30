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
    double lon, lat, temp, wind_speed, wind_deg, pressure;
    int clouds, humidity;
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
        goto e_fail;
    }

    if (!(node = n_json_find_object(root, "main"))) {
        goto e_fail;
    }
    weather->main = strdup(node->u.string.ptr);

    if (!(node = n_json_find_object(root, "name"))) {
        goto e_fail;
    }
    weather->name = strdup(node->u.string.ptr);

    if (!(node = n_json_find_object(root, "temp"))) {
        goto e_fail;
    }
    weather->temp = node->u.dbl;

    if (!(node = n_json_find_object(root, "speed"))) {
        goto e_fail;
    }
    weather->wind_speed= node->u.dbl;

    if (!(node = n_json_find_object(root, "all"))) {
        goto e_fail;
    }
    weather->clouds = node->u.integer;

    if (!(node = n_json_find_object(root, "humidity"))) {
        goto e_fail;
    }
    weather->humidity = node->u.integer;

    if (!(node = n_json_find_object(root, "pressure"))) {
        goto e_fail;
    }
    weather->pressure = node->u.dbl;

    json_value_free(root);
    return weather;
    
e_fail:
    free(weather);
    return NULL;
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
    const char * city_name = NULL;
    char * json_payload = NULL;
    struct weather_s * weather = NULL;

    city_name = get_city_name(plugin->irc->message.trailing);
    if (city_name == NULL) {
        sprintf(response, "PRIVMSG %s :Which city?", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        goto e_cleanup;
    }

    json_payload = get_json_weather_data(city_name);
    
    weather = parse_json(json_payload);
    
    if (weather == NULL) {
        sprintf(response, "PRIVMSG %s :No data could have been read!", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        goto e_cleanup;
    }
   
    snprintf(response, 512, 
        "PRIVMSG %s :%s (%s), Temp: %.2f \u00b0C, Wind speed: %.2f km/h, Cloud ratio: %d%%, Humidity: %d%%, Pressure: %.2f hPa", 
        plugin->irc->from, 
        weather->name, 
        weather->main, 
        weather->temp - 273.15, 
        MPS_TO_KMH(weather->wind_speed), 
        weather->clouds, 
        weather->humidity, 
        weather->pressure
    );
    plugin->send_message(plugin->irc, response);

e_cleanup:
    weather_free(weather);
    free(json_payload);
    
    return;
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

