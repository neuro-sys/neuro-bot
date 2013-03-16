#!/bin/bash

LIBS="jansson libcurl glib-2.0 gio-2.0 python-2.7"
CFLAGS="`pkg-config $LIBS --cflags` -I."
LDFLAGS="`pkg-config $LIBS --libs`"

gcc -c *.c modules/*.c $CFLAGS
gcc *.o -o ircclient $LDFLAGS



