## neurobot
Uriel is an IRC bot written in C with an extensible plugin architecture in mind. Plugins can be loaded at runtime without need to restart the application. A container plugin also may load other plugins and act as a plugin proxy -- ideally for the purpose of binding plugins written in other languages. For instance, there is one python plugin manager which is itself a plugin, and loads python plugins. Plugins for other languages are planned.

###### Features:
 - Admin management
 - Autojoin channels
 - Reconnect on disconnect
 - Extensible with plugins
 - No third party dependencies

###### How to build & run:
	$ make && ./neurobot

###### Dependencies: 
POSIX (for pthreads).

##### Some of the plugins may need the libraries below for their internal use.
 - Python (Required by the python plugin manager)
 - *[cURL](http://curl.haxx.se/libcurl/)* 
 
###### Build status:
[![Build Status](https://secure.travis-ci.org/neuro-sys/neuro-bot.png)](http://travis-ci.org/neuro-sys/neuro-bot)

