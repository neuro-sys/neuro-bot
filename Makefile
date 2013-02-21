CC = gcc
CFLAGS += $(shell pkg-config --cflags glib-2.0 jansson)
CFLAGS += $(shell curl-config --cflags)
export CFLAGS += -I. # all includes relative to base in subfolders
LDFLAGS += $(shell pkg-config --libs glib-2.0 jansson) 
export LDFLAGS += $(shell curl-config --libs)

SOURCES = channel.c \
	  curl_wrap.c \
	  irc.c \
	  log.c \
	  main.c \
	  network.c \
	  session.c \
	  user.c \
	  config.c


MOD_DIR = ./modules
MOD_OBJ = $(MOD_DIR)/*.o

ifeq ($(OS),Windows_NT)
SOURCES += socket_win.c
else
SOURCES += socket_unix.c
endif

ifdef DEBUG
	export DEBUGFLAG = -g
endif

export WARNINGFLAGS = -Wall

OBJECTS    = $(SOURCES:.c=.o)

all: irc-client 

irc-client : $(OBJECTS) modules
	$(CC) $(DEBUGFLAG) $(LDFLAGS) $(OBJECTS) $(MOD_OBJ) -o irc-client

.c.o:
	$(CC) -c $(DEBUGFLAG) $(WARNINGFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f irc-client $(OBJECTS); $(MAKE) --directory=$(MOD_DIR) clean

.PHONY: modules

modules:	
		$(MAKE) --directory=$(MOD_DIR)
