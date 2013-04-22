CC			= gcc
LDFLAGS		= -lm 
LDFLAGS		+= -Wall -g -O0 -Wl,--no-undefined --shared -fPIC

DEPS		= neurobotapi.o json.o

all: mod_wiki.so mod_youtube.so mod_title.so mod_time.so mod_whereis.so

mod_wiki.so: mod_wiki.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_wiki.o -o $@ 

mod_youtube.so: mod_youtube.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_youtube.o -o $@ 

mod_whereis.so: mod_whereis.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_whereis.o -o $@ 

mod_title.so: mod_title.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_title.o -o $@ 

mod_time.so: mod_time.o $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) mod_time.o -o $@ 

json.o: json.c
	$(CC) -Wl,--no-undefined --shared -fPIC -c json.c 

neurobotapi.o: neurobotapi.c
	$(CC) -Wl,--no-undefined --shared -fPIC -c neurobotapi.c 

clean:
	rm -fv *.o *.so *.pyc


