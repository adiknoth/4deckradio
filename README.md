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
  * make


Compile:
--------
  * For gstreamer-1.0, run
    `cd gstreamer && make`

  * For gstreamer-0.10, run
    `cd gstreamer && make OLDGSTREAMER=1`
    

Run:
----
    `./4deckradio`


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
