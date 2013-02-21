ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = ircclient
ircclient_SOURCES = main.c \
                    channel.c \
		    config.c \
		    curl_wrap.c \
		    irc.c \
		    log.c \
		    network.c \
		    session.c \
		    user.c \
		    py_wrap.c
ircclient_SOURCES += socket_unix.c
ircclient_SOURCES += socket_win.c
ircclient_LDADD = modules/libmodules.la

ircclient_CFLAGS = $(AM_CFLAGS) $(IRCCLIENT_CFLAGS) $(GLIB_CFLAGS) $(PYTHON_CFLAGS)
ircclient_LDADD += $(IRCCLIENT_CFLAGS) $(GLIB_LIBS) $(CURL_LIBS) $(JANSSON_LIBS) $(PYTHON_LIBS)

SUBDIRS = modules
