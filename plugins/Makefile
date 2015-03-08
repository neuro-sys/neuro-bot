CC     = clang
CFLAGS = -Wall -g -O0 -lm

ifneq ($(OS), Windows_NT)
	CFLAGS		 	+= -fPIC
endif

all: plugin_python_manager.so plugin_git.so plugin_title.so plugin_wiki.so plugin_youtube.so \
	plugin_test.so plugin_mesai.so plugin_youtube_search.so plugin_weather.so plugin_map.so \
    plugin_gris.so plugin_oktay_lol.so plugin_seen.so plugin_rss.so

plugin_python_manager.so: plugin_python_manager.c
	$(CC) plugin_python_manager.c $(CFLAGS) $(shell pkg-config python-2.7 --cflags --libs) $(shell python-config --cflags --libs) --shared -o$@ -g -O0

plugin_git.so: plugin_git.c
	$(CC) plugin_git.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@

plugin_title.so: plugin_title.c
	$(CC) plugin_title.c utils/curl_wrapper.c $(CFLAGS) `pkg-config libcurl libxml-2.0 --cflags --libs` --shared -o$@

plugin_gris.so: plugin_gris.c
	$(CC) plugin_gris.c $(CFLAGS) ../socket.c --shared -o$@

plugin_wiki.so: plugin_wiki.c
	$(CC) plugin_wiki.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@

plugin_youtube.so: plugin_youtube.c
	$(CC) plugin_youtube.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@

plugin_youtube_search.so: plugin_youtube_search.c
	$(CC) plugin_youtube_search.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@

plugin_map.so: plugin_map.c
	$(CC) plugin_map.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@ -g -O0

plugin_weather.so: plugin_weather.c
	$(CC) plugin_weather.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@ -g -O0

plugin_test.so: plugin_test.c
	$(CC) plugin_test.c $(CFLAGS) --shared -o$@

plugin_oktay_lol.so: plugin_oktay_lol.c
	$(CC) plugin_oktay_lol.c $(CFLAGS) --shared -o$@

plugin_seen.so: plugin_seen.c
	$(CC) plugin_seen.c $(CFLAGS) `pkg-config sqlite3 --cflags --libs` --shared -o$@

plugin_rss.so: plugin_rss.c
	$(CC) plugin_rss.c utils/curl_wrapper.c $(CFLAGS) `pkg-config sqlite3 libxml-2.0 libcurl --cflags --libs` --shared -o$@

plugin_mesai.so: plugin_mesai.c
	$(CC) plugin_mesai.c $(CFLAGS) --shared -o$@

clean:
	rm -fv *.o *.so

