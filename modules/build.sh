#!/bin/bash

[ "$1" == "rm" ] && rm -vf *.o *.so *.pyc && exit

LIBS="jansson libcurl glib-2.0 gio-2.0 python-2.7"
CFLAGS="`pkg-config $LIBS --cflags` -I.. -g"
LDFLAGS="`pkg-config $LIBS --libs` -g"
gcc -fPIC -g -c *.c $CFLAGS

gcc -Wl,--no-undefined $LDFLAGS --shared mod_youtube.o -omod_youtube.so ../curl_wrap.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_wiki.o -omod_wiki.so ../curl_wrap.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_title.o -omod_title.so ../curl_wrap.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_time.o -omod_time.so ../curl_wrap.o


