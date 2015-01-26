## neurobot
neurobot is an IRC (Internet Relay Chat) bot written in C extensible with plugins written in C, Python and Java.

###### Build status:
[![Build Status](https://secure.travis-ci.org/neuro-sys/neuro-bot.png)](http://travis-ci.org/neuro-sys/neuro-bot)

###### Dependencies:
The bot core needs POSIX threads to run.

But the contained plugins may have other dependencies.

* python2
  * BeautifulSoup
  * urlgrabber
* libxml2
* libcurl

###### Plugins:

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

