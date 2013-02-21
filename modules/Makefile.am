noinst_LTLIBRARIES = libmodules.la
libmodules_la_SOURCES = mod_time.c \
                        mod_title.c \
			mod_youtube.c

libmodules_la_CFLAGS = $(GLIB_CFLAGS) \
                       $(AM_CFLAGS) \
		       $(IRCCLIENT_CFLAGS) \
		       $(CURL_CFLAGS) \
		       $(JANSSON_CFLAGS)
