CC			= gcc
CFLAGS		= -g -O0
LDFLAGS		= -fPIC -g -O0 -Wall -Wl,--no-undefined -shared
AFTERFLAGS      = -lm
DEPS		= neurobotapi.o json.o

all: mod_wiki.so mod_youtube.so mod_title.so mod_time.so mod_whereis.so

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -shared -c $< -o $@

mod_wiki.so: mod_wiki.o $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) mod_wiki.o -o $@ $(AFTERFLAGS)

mod_youtube.so: mod_youtube.o $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) mod_youtube.o -o $@ $(AFTERFLAGS)

mod_whereis.so: mod_whereis.o $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) mod_whereis.o -o $@ $(AFTERFLAGS)

mod_title.so: mod_title.o $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) mod_title.o -o $@ $(AFTERFLAGS)

mod_time.so: mod_time.o $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEPS) mod_time.o -o $@ $(AFTERFLAGS)

clean:
	rm -fv *.o *.so *.pyc


