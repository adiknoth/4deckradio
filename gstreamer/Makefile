TARGET = 4deckradio

%.o: %.c %.h
	gcc -g -std=c99 -c $< `pkg-config --libs --cflags gstreamer-1.0 gtk+-3.0` -D_POSIX_C_SOURCE=200809L

4deckradio: mygstreamer.o audio.o
	gcc -g -std=c99 audio.o mygstreamer.o `pkg-config --libs --cflags gstreamer-1.0 gtk+-3.0` -o $@

all: ${TARGET}

clean:
	rm -rf *.o ${TARGET}
