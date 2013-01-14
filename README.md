4deckradio
==========

Multi-deck media playback for radio stations

![Screenshot](http://adiknoth.github.com/4deckradio/images/4deckradio.png)

Still very early development

Prerequisites:
  * gstreamer-1.0 or 0.10 (development packages)
  * GTK-3.x (development packages)
  * [jackd](http://jackaudio.org)
  * make

Compile:
  * For gstreamer-1.0, run
    `cd gstreamer && make`

  * For gstreamer-0.10, run
    `cd gstreamer && make OLDGSTREAMER=1`
    
Run:
    `./4deckradio`
