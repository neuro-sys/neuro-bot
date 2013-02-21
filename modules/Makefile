CFLAGS += -I..

SOURCES = \
	  mod_title.c \
	  mod_youtube.c \
          mod_time.c \

OBJECTS    = $(SOURCES:.c=.o)

all: $(OBJECTS)

.c.o:
	$(CC) -c $(DEBUGFLAG) $(WARNINGFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)

