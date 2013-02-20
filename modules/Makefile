CC = gcc
CFLAGS += $(shell pkg-config --cflags glib-2.0 jansson)
CFLAGS += $(shell curl-config --cflags)
CFLAGS += -I$(shell pwd)/..

SOURCES = \
	  mod_title.c \
	  mod_youtube.c \
    mod_time.c \

ifdef DEBUG
	DEBUGFLAG = -g
endif

WARNINGFLAGS = -Wall

OBJECTS    = $(SOURCES:.c=.o)

all: $(OBJECTS)

.c.o:
	$(CC) -c $(DEBUGFLAG) $(WARNINGFLAGS) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS)

