#  Copyright (c) 2013 Karl Lindén <spray.sucks@gmail.com>
#  
#  Permission is hereby granted, free of charge, to any person obtaining
#  a copy of this software and associated documentation files (the
#  "Software"), to deal in the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to
#  permit persons to whom the Software is furnished to do so, subject to
#  the following conditions:
#  
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT.
#  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([4deckradio], [GIT], [https://github.com/adiknoth/4deckradio/issues])
AC_CONFIG_SRCDIR([gstreamer/audio.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

m4_ifndef([PKG_PROG_PKG_CONFIG],
    [m4_fatal([pkg-config is required to build 4deckradio])])
PKG_PROG_PKG_CONFIG([0.24])

# Checks for libraries.
PKG_CHECK_MODULES(
	[GTK],
	[gtk+-3.0 >= 3.0],
	[],
	[AC_MSG_ERROR([GTK+ of atleast version 3.0 is required to build 4deckradio])
		exit -1]
)

# Check for both gstreamer versions so we can make a decision of which
# to use later on
PKG_CHECK_MODULES(
	[OLD_GSTREAMER],
	[gstreamer-0.10],
	[have_old_gstreamer=yes],
	[have_old_gstreamer=no]
)
PKG_CHECK_MODULES(
	[GSTREAMER],
	[gstreamer-1.0],
	[have_gstreamer=yes],
	[have_gstreamer=no]
)

# Let the user request building against the old gstreamer by giving the
# --with-old-gstreamer argument on the commandline
# --without-old-gstreamer can be used to explicitly request
# gstreamer-1.0
AC_ARG_WITH(
	[old-gstreamer],
	AS_HELP_STRING(
		[--with-old-gstreamer],
		[Build against old gstreamer-0.10 (default: detect)]
	),
	[want_old_gstreamer=$withval],
	[want_old_gstreamer=detect]
)

# Determine the version of gstreamer to be used. Print an error if the
# requested version of gstreamer cannot be used.
# If no version is explicitly requested gstreamer-1.0 will be chosen in
# favor of gstreamer-0.10.
# If no version of gstreamer can be found, print an error.
AS_IF(
	[test x$want_old_gstreamer = xyes],
	[AS_IF(
		[test x$have_old_gstreamer = xyes],
		[with_old_gstreamer=yes],
		[AC_MSG_ERROR(
[Old GStreamer support was explicitly requested, but gstreamer-0.10
cannot be found.])
		exit -1]
	)],
	[test x$want_old_gstreamer = xno],
	[AS_IF(
		[test x$have_gstreamer = xyes],
		[with_old_gstreamer=no],
		[AC_MSG_ERROR(
[New GStreamer support was explicitly requested, but gstreamer-1.0
cannot be found.])
		exit -1]
	)]
	[AS_IF(
		[test x$have_gstreamer = xyes],
		[with_old_gstreamer=no],
		[AS_IF(
			[test "x$have_old_gstreamer" = "xyes"],
			[with_old_gstreamer=yes],
			[AC_MSG_ERROR([GStreamer is required to build 4deckradio])
			exit -1]
		)]
	)]
)
AM_CONDITIONAL([WITH_OLD_GSTREAMER], [test x$with_old_gstreamer = xyes])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h getopt.h stdint.h stdio.h stdlib.h string.h sys/stat.h sys/types.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([
	gstreamer/Makefile
	Makefile
])
AC_OUTPUT
