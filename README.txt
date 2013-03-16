In order to build, you need libcurl, libglib-2.0 (with libgio-2.0), jansson and python2.7

Basically pkg-config --list-all | grep 'libcurl\|glib\|gio\|jansson\|python' should list each.

user@host ~/
$ git clone https://github.com/neuro-sys/irc-bot.git
$ cd irc-bot
$ ./build.sh

For MS Windows Visual Studio, see ~/irc-bot/contrib/vstudio10/
For MS Windows Cygwin, it's all the same.

If autotools does not work, you may use ~/irc-bot/build_simple.sh

For writing command modules see ~/irc-bot/modules/README


Jansson:
You can have it from its official page http://www.digip.org/jansson/
The version I used at the time of writing this can be obtained directly here:
http://www.digip.org/jansson/releases/jansson-2.4.tar.gz

