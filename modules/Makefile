CC			= gcc
LDFLAGS		= -lm
LDFLAGS		+= -fPIC -Wall -g -O0 -Wl,--no-undefined --shared

DEPS		= json.o neurobotapi.o

all: mod_wiki.so mod_youtube.so mod_title.so mod_time.so

mod_wiki.so: mod_wiki.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_wiki.o -o $@ 

mod_youtube.so: mod_youtube.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_youtube.o -o $@ 

mod_title.so: mod_title.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_title.o -o $@ 

mod_time.so: mod_time.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_time.o -o $@ 

clean:
	rm -fv *.o *.so *.pyc


