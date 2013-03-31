#!/bin/bash

[ "$1" == "rm" ] && rm *.o && exit

PYVER="python2.7"

LIBS="jansson libcurl"
PYCFLAGS=`$PYVER-config --cflags`
PYLDFLAGS=`$PYVER-config --libs`
CFLAGS="`pkg-config $LIBS --cflags` $PYCFLAGS -I. -g -O0 -U_FORTIFY_SOURCE"
LDFLAGS="`pkg-config $LIBS --libs` $PYLDFLAGS -ldl -O0"

gcc -Wall $CFLAGS -c *.c 
gcc -Wall $LDFLAGS *.o -o neurobot 

cd modules
./build.sh

