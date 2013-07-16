CFLAGS	   = -I. -Wall -g -O0 -lpthread 
LDFLAGS	   = -ldl -g -O0  -lpthread
OBJS	   =config.o \
		   irc.o \
		   socket.o \
		   py_wrap.o \
		   session.o \
		   irc_parser.o \
		   irc_plugin.o \
		   plugin.o\
		   main.o

MOD_DIR    = ./modules
TEST_DIR   = ./tests
DEPS	   = python-2.7
CFLAGS	   += $(shell pkg-config $(DEPS) --cflags)
LDFLAGS    += $(shell pkg-config $(DEPS) --libs)
CFLAGS     += -O0

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: neurobot unit_tests

neurobot: $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -fv $(OBJS) neurobot; $(MAKE) --directory=$(MOD_DIR) clean; $(MAKE) --directory=$(TEST_DIR) clean;

.PHONY: unit_tests 

#modules:
#	$(MAKE) --directory=$(MOD_DIR)

#unit_tests:
#	$(MAKE) --directory=$(TEST_DIR)


