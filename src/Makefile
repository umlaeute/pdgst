EXT=pd_linux

current: $(EXT)

NAME=pdgst
SOURCES=$(sort $(filter %.c, $(wildcard *.c)))
TARGETS = $(SOURCES:.c=.o)

# ----------------------- LINUX i386 -----------------------

.SUFFIXES: .$(EXT)

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer -fPIC \
    -Wall -W -Wshadow -Wstrict-prototypes $(shell pkg-config --cflags gstreamer-0.10) \
    -Wno-unused -Wno-parentheses -Wno-switch $(CFLAGS)

LINUXINCLUDE =  -I../../../pd/src

LINUXLDFLAGS = -lc -lm  $(shell pkg-config --libs gstreamer-0.10)

$(EXT): $(NAME).$(EXT)

$(NAME).$(EXT): $(TARGETS)
	$(CC) -shared -o $*.$(EXT) $^ $(LINUXLDFLAGS)
	echo strip --strip-unneeded $*.pd_linux

$(TARGETS): %.o : %.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c

clean:
	rm -f *.o *.pd_* so_locations
