CC			= gcc
LDFLAGS		= -lm 
LDFLAGS		+= -Wall -g -O0 -Wl,--no-undefined --shared -fPIC

DEPS		= neurobotapi.o

all: mod_wiki.so mod_youtube.so mod_title.so mod_time.so mod_whereis.so

mod_title.so: mod_title.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_title.o -o $@ 

mod_time.so: mod_time.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_time.o -o $@ 

neurobotapi.o: neurobotapi.c
	$(CC) -shared -c neurobotapi.c 

clean:
	rm -fv *.o *.so *.pyc


