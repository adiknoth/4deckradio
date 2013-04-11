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

# Enforce the C99 standard
AM_CPPFLAGS = -std=c99

bin_PROGRAMS = 4deckradio
4deckradio_SOURCES =	audio.c \
						audio.h \
						mygstreamer.c \
						mygstreamer.h

4deckradio_CFLAGS = $(GTK_CFLAGS)

4deckradio_LDADD = $(GTK_LIBS)

if WITH_OLD_GSTREAMER
4deckradio_CFLAGS += $(OLD_GSTREAMER_CFLAGS)
4deckradio_LDADD += $(OLD_GSTREAMER_LIBS)
else
4deckradio_CFLAGS += $(GSTREAMER_CFLAGS)
4deckradio_LDADD += $(GSTREAMER_LIBS)
endif
