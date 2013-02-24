#!/bin/bash

gcc -O0 *.c modules/*.c `pkg-config.exe jansson libcurl glib-2.0 gio-2.0 --libs --cflags` `python2.7-config --libs --cflags` -g -I. -oircclient -O0


