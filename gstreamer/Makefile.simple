TARGET = 4deckradio

GSTREAMER = gstreamer-1.0

ifeq ($(OLDGSTREAMER),1)
	GSTREAMER = gstreamer-0.10
endif

GSTREAMER_FLAGS = `pkg-config --libs --cflags ${GSTREAMER}`

MY_INCLUDES = ${GSTREAMER_FLAGS} `pkg-config --libs --cflags gtk+-3.0`

%.o: %.c *.h
	gcc -g -std=c99 -c $< ${MY_INCLUDES} -D_POSIX_C_SOURCE=200809L

4deckradio: mygstreamer.o audio.o
	gcc -g -std=c99 audio.o mygstreamer.o ${MY_INCLUDES} -o $@

all: ${TARGET}

clean:
	rm -rf *.o ${TARGET}
