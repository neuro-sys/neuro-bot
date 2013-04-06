#!/bin/bash

[ "$1" == "rm" ] && rm -vf *.o *.so *.pyc && exit

gcc -fPIC -Wall -g -O0 -c *.c 

gcc -Wl,--no-undefined --shared mod_youtube.o -omod_youtube.so neurobotapi.c json.c -lm
gcc -Wl,--no-undefined --shared mod_wiki.o -omod_wiki.so neurobotapi.c json.c -lm
gcc -Wl,--no-undefined --shared mod_title.o -omod_title.so neurobotapi.c json.c -lm
gcc -Wl,--no-undefined --shared mod_time.o -omod_time.so neurobotapi.c json.c -lm



