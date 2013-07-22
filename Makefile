CFLAGS	   = -I. -Wall -g -O0 
LDFLAGS	   = -ldl -g -O0  -lpthread
OBJS	   =config.o \
		   irc.o \
		   socket.o \
		   session.o \
		   irc_parser.o \
		   irc_plugin.o \
		   plugin.o\
		   main.o

MOD_DIR    = ./modules
TEST_DIR   = ./tests
CFLAGS     += -O0

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: neurobot unit_tests

neurobot: $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -fv $(OBJS) neurobot; $(MAKE) --directory=$(MOD_DIR) clean; $(MAKE) --directory=$(TEST_DIR) clean;

.PHONY: unit_tests test_plugin test_irc_parser test_config

test_plugin: plugin.c plugin.h
	$(CC) plugin.c socket.c -DTEST_PLUGIN -o $@ && ./$@ && rm -fv $@ 

test_irc_parser: irc_parser.c irc_parser.h
	$(CC) irc_parser.c -DTEST_IRC_PARSER -o $@ && ./$@ && rm -fv $@

test_config: config.c config.h
	$(CC) config.c -DTEST_CONFIG -o $@ && ./$@ && rm -fv $@

unit_tests:
	$(MAKE) test_plugin test_irc_parser test_config


#modules:
#	$(MAKE) --directory=$(MOD_DIR)

#unit_tests:
#	$(MAKE) --directory=$(TEST_DIR)


