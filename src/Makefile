EXT=pd_linux
CC=gcc 

current: $(EXT)

NAME=pdgst
SOURCES=$(sort $(filter %.c, $(wildcard *.c)))
TARGETS = $(SOURCES:.c=.o)

GST_CFLAGS=$(shell pkg-config --cflags gstreamer-0.10)
GST_LDFLAGS=$(shell pkg-config --libs gstreamer-0.10)

# ----------------------- LINUX i386 -----------------------

.SUFFIXES: .$(EXT)

LINUXCFLAGS = -g -O2 -fPIC -DPD $(GST_CFLAGS) $(CFLAGS)
LINUXINCLUDE =  -I../../../pd/src
LINUXLDFLAGS = -lc -lm  $(GST_LDFLAGS)

$(EXT): $(NAME).$(EXT)

$(NAME).$(EXT): $(TARGETS)
	$(CC) -shared -export_dynamic $(LINUXLDFLAGS) -o $*.$(EXT) $^

$(TARGETS): %.o : %.c pdgst.h Makefile
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c

clean:
	rm -f *.o *.pd_* so_locations
