4deckradio
==========

Multi-deck media playback for radio stations

![Screenshot](http://adiknoth.github.com/4deckradio/images/4deckradio.png)

Still very early development

Prerequisites:
--------------
  * gstreamer-1.0 or 0.10 (development packages)
  * GTK-3.x (development packages)
  * [jackd](http://jackaudio.org)
  * make and optionally autotools


Compile:
--------
There are two ways to compile 4deckradio.

First, the plain and simple approach without autotools:

  * For gstreamer-1.0, run
    `cd gstreamer && make -f Makefile.simple`

  * For gstreamer-0.10, run
    `cd gstreamer && make OLDGSTREAMER=1 -f Makefile.simple`

For packagers, there's the more sophisticated autotools-based approach:

    ./autogen.sh
    ./configure
    
    ./configure will automatically build against gstreamer-1.0 if it can
    be found, else it will build against gstreamer-0.10. (If none can be
    found it will print out a warning.)
    
    If you want to excplicitly specify which version of gstreamer to
    build against you can pass --with-old-gstreamer or
    --without-old-gstreamer to ./configure. For example:

  * For gstreamer-1.0
    ./configure --without-old-gstreamer

  * For gstreamer-0.10
    ./configure --with-old-gstreamer

    Then run:
    make
    make install
    
You don't have to install 4deckradio, you can also run the binary from
the build directory.

Run:
----
    ./4deckradio [options]
    
                -f      fullscreen
                -a      autoconnect to jackd
                -h      help (this message)



Usage: (rudimentary documentation)
-----------------------------------

You select a file and press play. You could also press F9 .. F12 to
start/stop the decks. Or you own a studio surface which sends joystick
button press/release events.

If you have a USB joystick, simply connect it. Press Button 1, and it
plays deck 1, release button 1, and it stops. Of course, you can also
press Buttons 1-4 simultaneously to have all four decks playing at once.

Selecting a new file while one is playing queues the new file. Press
stop or wait for the current one to finish to actually play it. (it's a
safety for radio stations, so they're not accidentally stopping the
current playback).

Same for stop: program only quits if you stop all four decks and then
press Ctrl+q. Well, the window-close button is a shortcut, but it
wouldn't be visible in fullscreen mode.
