## neurobot
neurobot is an IRC (Internet Relay Chat) bot written in C extensible with plugins written in C, Python and Java.

###### Build status:
[![Build Status](https://secure.travis-ci.org/neuro-sys/neuro-bot.png)](http://travis-ci.org/neuro-sys/neuro-bot)

###### Plugins:

* git
  * Tracks github repository commits history. 
* gris
  * Using google reverse image, tries to guess what the image is for a URL posted.
* python_manager
  * Binds to Python interpreter for plugins written in Python. 
* java_manager
  * Fires up a JVM for plugins written in Java.
* map
  * Prints a small ASCII world map, and marks the designated place on it. 
* mesai
  * Displays the remaining time for when the work is over. 
* test
  * Stub skeleton for new plugins to be written. 
* title
  * Prints the title of a URL. 
* weather
  * Using openweathermap, prints weather data of a given location. 
* wiki
  * Prints the description of a wiki article. 
* youtube
  * Prints information on a youtube video. 
* youtube_search
  * Searches youtube for videos. 
* eksi
  * Searches and prints Eksi Sozluk entries.
* seen 
  * Records the last sentence someone in a room said with a timestamp, and queries it.

###### How to build & run:
Build
```bash
$ make
```
Edit configuration
```bash
$ vim neurobot.conf
```
Run
```bash
$ ./neurobot
```

