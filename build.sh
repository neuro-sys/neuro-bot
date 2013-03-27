#!/bin/bash

[ "$1" == "rm" ] && rm *.o && exit

PYVER="python2.7"

LIBS="jansson libcurl glib-2.0 gio-2.0"
PYCFLAGS=`$PYVER-config --cflags`
PYLDFLAGS=`$PYVER-config --libs`
CFLAGS="`pkg-config $LIBS --cflags` $PYCFLAGS -I. -g"
LDFLAGS="`pkg-config $LIBS --libs` $PYLDFLAGS -ldl -g"

gcc -Wall $CFLAGS -c *.c 
gcc -Wall $LDFLAGS *.o -o neurobot 

cd modules
./build.sh

