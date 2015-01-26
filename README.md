## neurobot
neurobot is an IRC (Internet Relay Chat) bot written in C extensible with plugins written in C, Python and Java.

##### Dependencies:
The bot core needs POSIX threads to run.
```bash
[neuro@localhost neuro-bot]$ ldd ./neurobot
        linux-vdso.so.1 (0x00007fffb7bfd000)
        libdl.so.2 => /usr/lib/libdl.so.2 (0x00007ff35b97d000)
        libpthread.so.0 => /usr/lib/libpthread.so.0 (0x00007ff35b761000)
        libc.so.6 => /usr/lib/libc.so.6 (0x00007ff35b3be000)
        /lib64/ld-linux-x86-64.so.2 (0x00007ff35bb81000)
```

Plugins may have other dependencies. The present plugins require the following libraries:

##### Plugins:

* plugins/plugin_git.c
  * Tracks github repository commits history. 
* plugins/plugin_gris.c
  * Using google reverse image, tries to guess what the image is for a URL posted.
* plugins/plugin_java_manager.c
  * Fires up a JVM for plugins written in Java.
* plugins/plugin_map.c
  * Prints a small ASCII world map, and marks the designated place on it. 
* plugins/plugin_mesai.c
  * Displays the remaining time for when the work is over. 
* plugins/plugin_python_manager.c
  * Binds to Python interpreter for plugins written in Python. 
* plugins/plugin_test.c
  * Stub skeleton for new plugins to be written. 
* plugins/plugin_title.c
  * Prints the title of a URL. 
* plugins/plugin_weather.c
  * Using openweathermap, prints weather data of a given location. 
* plugins/plugin_wiki.c
  * Prints the description of a wiki article. 
* plugins/plugin_youtube.c
  * Prints information on a youtube video. 
* plugins/plugin_youtube_search.c
  * Searches youtube for videos. 
* plugins/mod_eksi.py
  * Searches and prints Eksi Sozluk entries.
* plugins/mod_example.py
  * Stub skeleton for python plugins.

###### How to build & run:
To build the bot itself:
```bash
$ make
```
To build plugins:
```bash
$ make plugins
```

Edit config file:
```bash
$ vim neurobot.conf
```

Run:

```bash
$ ./neurobot
```

###### Build status:
[![Build Status](https://secure.travis-ci.org/neuro-sys/neuro-bot.png)](http://travis-ci.org/neuro-sys/neuro-bot)

