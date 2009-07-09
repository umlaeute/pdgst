EXT?=pd_linux
CC?=gcc 
PDSRC?=../../../pd/src

vpath %.h ../include/pdgst

current: $(EXT)

NAME=pdgst
SOURCES=$(sort $(filter %.c, $(wildcard *.c */*.c)))
TARGETS = $(SOURCES:.c=.o)

GST_CFLAGS=$(shell pkg-config --cflags gstreamer-0.10)
GST_LDFLAGS=$(shell pkg-config --libs gstreamer-0.10)

GST_PLUGIN_CFLAGS=-DPDGST_PLUGIN $(shell pkg-config --cflags gstreamer-plugins-base-0.10)
GST_PLUGIN_LDFLAGS=-lgstapp-0.10 $(shell pkg-config --libs gstreamer-plugins-base-0.10)

# ----------------------- LINUX i386 -----------------------

.SUFFIXES: .$(EXT)

LINUXCFLAGS = -g -O2 -fPIC -DPD $(GST_CFLAGS) $(GST_PLUGIN_CFLAGS) $(CFLAGS)
LINUXINCLUDE =  -I$(PDSRC) -I../include
LINUXLDFLAGS = -lc -lm  $(GST_LDFLAGS) $(GST_PLUGIN_LDFLAGS)

$(EXT): $(NAME).$(EXT)

$(NAME).$(EXT): $(TARGETS)
	$(CC) -shared $(LINUXLDFLAGS) -o $*.$(EXT) $^

$(TARGETS): %.o : %.c pdgst.h Makefile
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c

clean:
	rm -f *.o *.pd_* */*.o */*.pd_* so_locations
