CC = gcc
CFLAGS += $(shell pkg-config --cflags glib-2.0 jansson)
CFLAGS += $(shell curl-config --cflags)
LDFLAGS += $(shell pkg-config --libs glib-2.0 jansson) 
LDFLAGS += $(shell curl-config --libs)

SOURCES = channel.c \
	  curl_wrap.c \
	  irc.c \
	  log.c \
	  main.c \
	  mod_title.c \
	  mod_youtube.c \
	  network.c \
	  session.c \
	  user.c \
	  config.c
ifeq ($(OS),Windows_NT)
SOURCES += socket_win.c
else
SOURCES += socket_unix.c
endif

ifdef DEBUG
	DEBUGFLAG = -g
endif

WARNINGFLAGS = -Wall

OBJECTS    = $(SOURCES:.c=.o)

all: irc-client

irc-client: $(OBJECTS)
	$(CC) $(DEBUGFLAG) $(LDFLAGS) $(OBJECTS) -o irc-client

.c.o:
	$(CC) -c $(DEBUGFLAG) $(WARNINGFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f irc-client $(OBJECTS)
