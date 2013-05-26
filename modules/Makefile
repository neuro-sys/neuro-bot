CC			= gcc
CFLAGS		= -g -O0
LDFLAGS		= -fPIC -g -O0 -Wall -Wl,--no-undefined -shared
AFTERFLAGS  = -lm -O0
DEPS		= neurobotapi.o json.o
UTILS		= curl_wrap.o utils.o
UTIL_LDFLAGS = $(shell pkg-config libcurl --libs)

all: mod_wiki.so mod_youtube.so mod_title.so mod_time.so mod_whereis.so mod_git.so

# UTILITIES 
curl_wrap.o: curl_wrap.c curl_wrap.h
	$(CC) -c $< -o $@ -fPIC -shared $(shell pkg-config libcurl --cflags)

utils.o: utils.c utils.h
	$(CC) -c $< -o $@ -fPIC -shared
# PLUGINS

%.o: %.c
	$(CC) -c $< -o $@ -fPIC -shared -g

mod_wiki.o: mod_wiki.c
	$(CC) -c $< -o $@ -fPIC -shared

mod_wiki.so: mod_wiki.o $(UTILS) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) $(UTILS) mod_wiki.o -o $@  $(UTIL_LDFLAGS) $(AFTERFLAGS)

mod_youtube.so: mod_youtube.o $(UTILS) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) $(UTILS) mod_youtube.o -o $@ $(UTIL_LDFLAGS) $(AFTERFLAGS)

mod_title.so: mod_title.o $(UTILS) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) $(UTILS) mod_title.o -o $@ $(UTIL_LDFLAGS) $(AFTERFLAGS)

mod_whereis.so: mod_whereis.o $(UTILS) $(DEPS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(UTILS) $(DEPS) mod_whereis.o -o $@ $(UTIL_LDFLAGS) $(AFTERFLAGS)

mod_time.so: mod_time.o $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) mod_time.o -o $@ $(AFTERFLAGS)

mod_git.so: mod_git.o $(UTILS) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) $(UTILS) mod_git.o -o $@ $(UTIL_LDFLAGS) $(AFTERFLAGS)

clean:
	rm -fv *.o *.so *.pyc


