#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([irc-client], [0.01], [https://github.com/neuro-sys/irc-client/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2 subdir-objects])
AC_CONFIG_SRCDIR([channel.c])
AC_CONFIG_HEADERS([aconfig.h])
AC_CONFIG_FILES([
  Makefile
  modules/Makefile
])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR

LT_INIT

AC_LANG([C])

PKG_PROG_PKG_CONFIG

IRCCLIENT_LIBS=""
IRCCLIENT_CFLAGS=""
AC_SUBST(IRCCLIENT_LIBS)
AC_SUBST(IRCCLIENT_CFLAGS)

# Checks for header files.
AC_CHECK_HEADERS([netdb.h stdlib.h string.h sys/socket.h])

PKG_CHECK_MODULES([GLIB], [glib-2.0 gio-2.0],,
		[AC_MSG_ERROR([GLib is required])])
PKG_CHECK_MODULES([JANSSON], [jansson],,
                  [AC_MSG_ERROR([Jansson is required])])
AC_CHECK_CURL([7], [], [ AC_MSG_ERROR([No curl library found.  You can get it at http://curl.haxx.se]) ])
PYTHON_CFLAGS="`python2.7-config --cflags`"
PYTHON_LIBS="`python2.7-config --libs`"
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

AC_CANONICAL_HOST
have_windows="no" 
case "$host_os" in 
   mingw*) 
      have_windows="yes" 
   ;; 
esac 

if test x$have_windows = xyes; then
	AC_DEFINE(HAVE_WINDOWS, 1, [We are on Windows])
else
	AC_DEFINE(HAVE_UNIX, 1, [We are on Unix])
fi
AC_SUBST(HAVE_WINDOWS)
AC_SUBST(HAVE_UNIX)

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset socket strdup strspn])

AC_OUTPUT
