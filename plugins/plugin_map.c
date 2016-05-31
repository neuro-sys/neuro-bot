#include "plugin_client.h"

#include "utils/curl_wrapper.h"
#include "utils/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define WEATHER_API "http://api.openweathermap.org/data/2.5/weather?APPID=58574ae90c3e027c00b2e7a22268f6e9&q="

#define WIDTH 	        36
#define HEIGHT	        18
#define MAP_BUF_LEN 	WIDTH*HEIGHT+HEIGHT

struct plugin_t * plugin;

struct gps_s {
    double lon, lat;
    char * name;
    int http_status;
};

static time_t last_clock;

static void gps_free(struct gps_s * gps)
{
    if (gps == NULL)
        return;

    if (gps->name)
        free(gps->name);

    free(gps);
}

/*
 * This is the map source. Using sed this is converted
 * to the bitmap array below.
 */

//...|...|...|...|...|...#...|...|...|...|...|...|
//...|...|xxx|xx.|xxx|x..#.x.|..x|...|xx.|..x|...|
//xxx|xxx|xxx|x..|.x.|.x.#.xx|xxx|xxx|xxx|xxx|xxx|
//---|---|---|---|---|---#---|---|---|---|---|---|
//.xx|.xx|xxx|xxx|...|...#.xx|.xx|xxx|xxx|xx.|.x.|
//...|..x|xxx|xx.|x..|...#xxx|.xx|xxx|xxx|xxx|...|
//...|..x|xxx|xx.|...|...#.x.|.xx|xxx|xxx|xxx|...|
//---|---|---|---|---|---#---|---|---|---|---|---|
//...|...|xxx|.x.|...|..x#xxx|xxx|xxx|xxx|...|...|
//...|...|.xx|x..|...|.xx#xxx|xxx|.xx|.x.|x..|...|
//...|...|...|.xx|xx.|.xx#xxx|xx.|...|.xx|x..|...|
//===|===|===|===|===|===#===|===|===|===|===|===|
//...|...|...|.xx|xxx|...#.xx|x..|...|.xx|xxx|...|
//...|...|...|.xx|xxx|...#.xx|xxx|...|...|xxx|...|
//...|...|...|..x|xx.|...#.xx|x..|...|..x|xxx|...|
//---|---|---|---|---|---#---|---|---|---|---|---|
//...|...|...|.xx|x..|...#..x|...|...|..x|xxx|..x|
//...|...|...|.xx|...|...#...|...|...|...|..x|.xx|
//...|...|...|.xx|...|...#...|...|...|...|...|...|
//---|---|---|---|---|---#---|---|---|---|---|---|
//...|...|...|...|...|...#...|...|...|...|...|...|
//...|...|...|...|...|...#...|...|...|...|...|...|
//...|...|...|...|...|...#...|...|...|...|...|...|
//---|---|---|---|---|---#---|---|---|---|---|---|


const char *world_map[] = 
{
    "                                    ",
    "      ..... ....   .   .   ..   .   ",
    "..........   .  .  .................",
    " .. ........       .............  . ",
    "     ...... .     ... ...........   ",
    "     ......       .  ............   ",
    "      ... .      .............      ",
    "       ...      ........ .. . .     ",
    "          ....  .......     ...     ",
    "          .....    ...      .....   ",
    "          .....    .....      ...   ",
    "           ...     ...       ....   ",
    "          ...       .        ....  .",
    "          ..                    . ..",
    "          ..                        ",
    "                                    ",
    "                                    ",
    "                                    ",
};


// [0, 360] latitude
// [0, 180] longtitude
// [0, 36] map lat
// [0, 18] map long

static void convert_world_coord_to_map_coord(int lat, int lon, int * mx, int * my)
{
    *mx = ((lat + 180.) / 360.) * WIDTH;
    *my = ((-lon + 90.) / 180.) * HEIGHT;

    if (*mx >= WIDTH) 	*mx = WIDTH-1;
    if (*mx < 0) 		*mx = 0;
    if (*my >= HEIGHT)	*my = HEIGHT-1;
    if (*my <0)			*my = 0;
}

static char * get_map(int lon, int lat)
{
    int x, y;
    int mx, my;
    char *map_buf = malloc(MAP_BUF_LEN); // +HEIGHT is for newline chars.
    int i = 0;

    convert_world_coord_to_map_coord(lon, lat, &mx, &my);

    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            if (x == mx && y == my) {
                map_buf[i++] = '@';
            } else {
                map_buf[i++] = world_map[y][x];
            }
        }
        map_buf[i++] = '\n';
    }

    return map_buf;
}

static void send_map(char * map_buf)
{
    int i;
    char response_line[512];
    int is_empty_line = 1;

    response_line[0] = 0;

    for (i = 0; i < MAP_BUF_LEN; i++) {
        char c = map_buf[i];

        if (i == (MAP_BUF_LEN / 2)) {
            sleep(3); /* Avoid flooding. */
        }

        if (c == '\n' || i == 0) {
            if (is_empty_line) {
                continue;
            }
            is_empty_line = 1;
            plugin->send_message(plugin->irc, response_line);
            usleep(500*1000); /* Avoid flooding. */
            sprintf(response_line, "PRIVMSG %s :", plugin->irc->from);
        } else {
            char append[2];
            if (map_buf[i] != ' ') is_empty_line = 0;

            sprintf(append, "%c", map_buf[i]);
            strcat(response_line, append);
        }
    }
}


static struct gps_s * parse_json(char * json_payload)
{
    struct gps_s * gps;
    json_value * root, * node;

    gps = malloc(sizeof (struct gps_s));
    memset(gps, 0, sizeof(*gps));

    if (!(root = json_parse(json_payload))) {
        goto e_fail;
    }

    if (!(node = n_json_find_object(root, "cod"))) {
        goto e_fail;
    }
    if ((gps->http_status = node->u.integer) != 200) {
        return gps;
    }

    if (!(node = n_json_find_object(root, "name"))) {
        goto e_fail;
    }
    gps->name = strdup(node->u.string.ptr);

    if (!(node = n_json_find_object(root, "lon"))) {
        goto e_fail;
    }
    gps->lon = node->u.dbl;

    if (!(node = n_json_find_object(root, "lat"))) {
        goto e_fail;
    }
    gps->lat = node->u.dbl;

    json_value_free(root);
    return gps;

e_fail:
    free(gps);
    puts(json_payload); // for error debugging
    return NULL;
}

static char * get_json_gps_data(const char * city_name)
{
    char url[512];
    char * ret;
    struct http_req * get; 

    snprintf(url, 512, "%s%s", WEATHER_API, city_name);

    get = curl_perform(url, NULL);
    if (!get->body)
        return NULL;

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

void run(int type)
{
    char response[512];
    const char * city_name = NULL;
    char * json_payload = NULL;
    struct gps_s * gps = NULL;
    char * map_buf = NULL;

    double diff_sec = difftime(time(NULL), last_clock);

    if (diff_sec < 15) {
        sprintf(response, "PRIVMSG %s :Wait %d seconds more...", plugin->irc->from, (int) (15 - diff_sec));
        plugin->send_message(plugin->irc, response);
        return;
    }
    last_clock = time(NULL);

    city_name = get_city_name(plugin->irc->message.trailing);
    if (city_name == NULL) {
        sprintf(response, "PRIVMSG %s :Which city?", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        goto e_cleanup;
    }

    json_payload = get_json_gps_data(city_name);
    if (json_payload == NULL) {
        sprintf(response, "PRIVMSG %s :No data could have been read!", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        goto e_cleanup;
    }

    gps = parse_json(json_payload);

    if (gps == NULL) {
        sprintf(response, "PRIVMSG %s :Could not parse the response!", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        goto e_cleanup;
    }

    if (gps->http_status != 200) {
        sprintf(response, "PRIVMSG %s :Not found.", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        goto e_cleanup;
    }

    sprintf(response, "PRIVMSG %s :[Incoming transmission...]", plugin->irc->from);
    plugin->send_message(plugin->irc, response);    

    map_buf = get_map(gps->lon, gps->lat);
    send_map(map_buf);

    sprintf(response, "PRIVMSG %s :%s, Lat: %.2f, Lon: %.2f", 
        plugin->irc->from,
        strcmp(gps->name, "") ? gps->name : city_name, // weather service can return empty name
        gps->lat,
        gps->lon
    );
    plugin->send_message(plugin->irc, response);

e_cleanup:
    gps_free(gps);
    if (json_payload)
        free(json_payload);
    if (map_buf)
        free(map_buf);

    return;
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "map";
    plugin->type       = PLUGIN_TYPE_COMMAND;

    return plugin;
}

