## neurobot
neurobot is an IRC bot written in C with extensibility in mind. Plugins can be written 
as either standalone shared libraries (e.g. .dll or .so), or python at the moment.

###### Features:
 - Admin management
 - Load or unoad plugins in runtime without having to restart the bot.
 - Run python code through the embedded python interpreter.

###### How to build & run:
	$ make && ./neurobot

###### Linkage dependencies: 
 - python2 (Optional for plugins)
 - *[cURL](http://curl.haxx.se/libcurl/)* (Optional for plugins)
 - *[Gnu Regex for Windows](http://gnuwin32.sourceforge.net/packages/regex.htm)* (<i>Not required in POSIX environments</i>)

###### Python Plugin support:
Define USE_PYTHON_MODULES in order to build with Python plugin support.

###### Build status:
[![Build Status](https://secure.travis-ci.org/neuro-sys/neuro-bot.png)](http://travis-ci.org/neuro-sys/neuro-bot)

