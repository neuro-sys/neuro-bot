CC			= gcc
CFLAGS		= -g -O0
LDFLAGS		= -g -O0 -Wall -Wl,--no-undefined --shared -lm

DEPS		= neurobotapi.o json.o

all: mod_wiki.so mod_youtube.so mod_title.so mod_time.so mod_whereis.so

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

mod_wiki.so: mod_wiki.o $(DEPS)
	$(CC) $(DEPS) $(CFLAGS) $(LDFLAGS) mod_wiki.o -o $@ 

mod_youtube.so: mod_youtube.o $(DEPS)
	$(CC) $(DEPS) $(CFLAGS) $(LDFLAGS) mod_youtube.o -o $@ 

mod_whereis.so: mod_whereis.o $(DEPS)
	$(CC) $(DEPS) $(CFLAGS) $(LDFLAGS) mod_whereis.o -o $@ 

mod_title.so: mod_title.o $(DEPS)
	$(CC) $(DEPS) $(CFLAGS) $(LDFLAGS) mod_title.o -o $@ 

mod_time.so: mod_time.o $(DEPS)
	$(CC) $(DEPS) $(CFLAGS) $(LDFLAGS) mod_time.o -o $@ 

clean:
	rm -fv *.o *.so *.pyc


