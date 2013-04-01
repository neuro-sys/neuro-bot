#!/bin/bash

[ "$1" == "rm" ] && rm -vf *.o *.so *.pyc && exit

PYVER="python2.7"

PYCFLAGS=`$PYVER-config --cflags`
PYLDFLAGS=`$PYVER-config --libs`
CFLAGS="`pkg-config $LIBS --cflags` $PYCFLAGS -g -O0 -U_FORTIFY_SOURCE"
LDFLAGS="`pkg-config $LIBS --libs` $PYLDFLAGS -ldl -g -O0 "

gcc -fPIC -g -c *.c $CFLAGS

gcc -Wl,--no-undefined $LDFLAGS --shared mod_youtube.o -omod_youtube.so neurobotapi.c json.c
gcc -Wl,--no-undefined $LDFLAGS --shared mod_wiki.o -omod_wiki.so neurobotapi.c json.c
gcc -Wl,--no-undefined $LDFLAGS --shared mod_title.o -omod_title.so neurobotapi.c json.c
gcc -Wl,--no-undefined $LDFLAGS --shared mod_time.o -omod_time.so neurobotapi.c json.c



