#!/bin/bash

[ "$1" == "rm" ] && rm -vf *.o *.so *.pyc && exit

PYVER="python2.6"

LIBS="jansson libcurl glib-2.0 gio-2.0"
PYCFLAGS=`$PYVER-config --cflags`
PYLDFLAGS=`$PYVER-config --libs`
CFLAGS="`pkg-config $LIBS --cflags` $PYCFLAGS -I.. -g -O0"
LDFLAGS="`pkg-config $LIBS --libs` $PYLDFLAGS -ldl -g -O0"

gcc -fPIC -g -c *.c $CFLAGS

gcc -Wl,--no-undefined $LDFLAGS --shared mod_youtube.o -omod_youtube.so ../curl_wrap.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_wiki.o -omod_wiki.so ../curl_wrap.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_title.o -omod_title.so ../curl_wrap.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_time.o -omod_time.so ../curl_wrap.o


