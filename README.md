## neurobot
neurobot is an IRC bot written in C with extensibility in mind. Plugins can be written 
as either standalone shared libraries (e.g. .dll or .so), or python at the moment.

###### Features:
 - Loading of plugins at runtime without having to restart the bot.
 - Plugins that are written in Python are run through the embedded Python interpreter.

###### How to build & run:
	$ make && ./neurobot

###### Dependencies: 
 - *[cURL](http://curl.haxx.se/libcurl/)*
 - python2 (Optional)
 - *[Gnu Regex for Windows](http://gnuwin32.sourceforge.net/packages/regex.htm)* (<i>Not required in POSIX environments</i>)

###### Python Plugin support:
Define USE_PYTHON_MODULES in order to build with Python plugin support.

###### Binaries:
 - *[neurobot-0.1.322.rar](https://dl.dropboxusercontent.com/u/28327717/neurobot-0.1.322.rar)* i686 Cygwin build <i>without python plugin support</i>.
 - *[neurobot-0.1.332.7z](https://dl.dropboxusercontent.com/u/28327717/neurobot-0.1.332.7z)* i686 MSVC build <i>without python plugin support</i>.
 - *[neurobot-0.1.332_python.7z](https://dl.dropboxusercontent.com/u/28327717/neurobot-0.1.332_python.7z)* i686 MSVC build <i>with python plugin support</i>.

###### Build status:
[![Build Status](https://secure.travis-ci.org/neuro-sys/neuro-bot.png)](http://travis-ci.org/neuro-sys/neuro-bot)

