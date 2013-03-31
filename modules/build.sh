#!/bin/bash

[ "$1" == "rm" ] && rm -vf *.o *.so *.pyc && exit

PYVER="python2.7"

LIBS="jansson libcurl"
PYCFLAGS=`$PYVER-config --cflags`
PYLDFLAGS=`$PYVER-config --libs`
CFLAGS="`pkg-config $LIBS --cflags` $PYCFLAGS -I.. -g -O0 -U_FORTIFY_SOURCE"
LDFLAGS="`pkg-config $LIBS --libs` $PYLDFLAGS -ldl -g -O0 -L."

gcc -fPIC -g -c *.c $CFLAGS

gcc -Wl,--no-undefined $LDFLAGS --shared mod_youtube.o -omod_youtube.so ../curl_wrap.o ../neurobotapi.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_wiki.o -omod_wiki.so ../curl_wrap.o ../neurobotapi.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_title.o -omod_title.so ../curl_wrap.o ../neurobotapi.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_time.o -omod_time.so ../curl_wrap.o ../neurobotapi.o
gcc -Wl,--no-undefined $LDFLAGS --shared mod_ddg.o -omod_ddg.so ../curl_wrap.o ../neurobotapi.o



