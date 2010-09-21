default: all
#############################################################
# videoIO makefile
#
# 2008, Georg Holzmann
# 2008, IOhannes m zmölnig
#############################################################

-include ../Make.config

GEM_SRC=../../../Gem/src
PD_SRC=../../../../pd/src
PDGST_SRC=../../include/
GST_CFLAGS=-DPDGST_PLUGIN $(shell pkg-config --cflags gstreamer-plugins-base-0.10 gstreamer-0.10)
GST_LDFLAGS=-lgstapp-0.10 $(shell pkg-config --libs gstreamer-plugins-base-0.10 gstreamer-0.10)

#############################################################
# build gem-external targets and objects

INCLUDE=-I$(PD_SRC) -I$(GEM_SRC) -I. -I$(PDGST_SRC)
LIBS=-lc -lm $(PD_LDFLAGS) $(GEM_LDFLAGS)  $(PDGST_LDFLAGS) $(GST_LDFLAGS)
CC_FLAGS=-c -Wall -O2 -fPIC -ffast-math -mmmx $(GST_CFLAGS)
LD_FLAGS=--export-dynamic -shared -o
EXTENSION=pd_linux

SOURCE := $(wildcard pix_*.cpp)
OBJECTS := $(SOURCE:.cpp=.o)
TMP := $(wildcard pix_*.cpp)
TARGETS := $(TMP:.cpp=.$(EXTENSION))

.PHONY: default all

# all builds gem-externals and plugins (gstreamer)

all: $(TARGETS)

$(TARGETS) : %.$(EXTENSION) : %.o pdgstGem.o
	g++ $(LD_FLAGS) $*.$(EXTENSION) $< pdgstGem.o $(LIBS)

pdgstGem.o $(OBJECTS) : %.o : %.cpp
	g++ $(CC_FLAGS) $(INCLUDE) -o "$*.o" -c "$*.cpp"


#############################################################
# clean and install

clean:
	rm -f *.o $(TARGETS)

#install:
#	install -d $(PD_INSTALL)
#	install -p $(TARGETS) $(PD_INSTALL)
#	install -d $(PD_HELP)
#	install -p ../help/*.pd $(PD_HELP)

# EOF
#############################################################
