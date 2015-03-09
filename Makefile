CC         = clang
CFLAGS	   = -I. -Wall -g -O0 
LDFLAGS	   = -ldl -g -O0  -lpthread
OBJS	   =config.o \
		   irc.o \
		   socket.o \
		   irc_parser.o \
		   plugin.o \
           channel.o \
            argv.o \
		   main.o

PLUGIN_DIR = ./plugins
CFLAGS     += -O0

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: neurobot plugins

neurobot: $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -fv $(OBJS) neurobot && $(MAKE) --directory=$(PLUGIN_DIR) clean;

.PHONY: test plugins

test_plugin: plugin.c plugin.h
	$(CC) plugin.c socket.c -DTEST_PLUGIN -pthread -ldl -o $@ && ./$@ && rm -fv $@ 

test_irc_parser: irc_parser.c irc_parser.h
	$(CC) irc_parser.c -DTEST_IRC_PARSER -o $@ && ./$@ && rm -fv $@

test_config: config.c config.h
	$(CC) config.c -DTEST_CONFIG -o $@ && ./$@ && rm -fv $@

test:
	$(MAKE) test_plugin test_irc_parser test_config test_irc_plugin

plugins:
	$(MAKE) --directory=$(PLUGIN_DIR)


