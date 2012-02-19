default: all
#############################################################
# pdgst makefile
#
# 2008, Georg Holzmann
# 2008-2012, IOhannes m zmölnig
#############################################################

-include ../Make.config

## assuming that your filesystem layout is like that:
### .../pd
### .../externals/Gem
### .../externals/pdgst
## (like the Pd-SVN layout)

GEM_SRC=../../../Gem/src
PD_SRC =../../../../pd/src
PDGST_SRC=../../include/
GST_CFLAGS=-DPDGST_PLUGIN $(shell pkg-config --cflags gstreamer-plugins-base-0.10 gstreamer-0.10)
GST_LIBS=-lgstapp-0.10 $(shell pkg-config --libs gstreamer-plugins-base-0.10 gstreamer-0.10)

#############################################################
# build gem-external targets and objects
EXT=pd_linux

SYSTEMCFLAGS = -g -O2 -fPIC
SYSTEMLIBS = -lc -lm
SYSTEMLDFLAGS = -fPIC -rdynamic -shared
SYSTEMINCLUDE =
SYSTEMOPTCFLAGS=-c -Wall -O2 -fPIC -ffast-math -mmmx

CXXFLAGS = -DPD $(SYSTEMCFLAGS) $(GST_CFLAGS) $(GST_PLUGIN_CFLAGS)
INCLUDE = -I$(PD_SRC) -I$(GEM_SRC) -I$(GEM_SRC)/deprecated -I. -I$(PDGST_SRC) -I../include $(SYSTEMINCLUDE)
LDFLAGS = $(SYSTEMLDFLAGS) $(GST_PLUGIN_LDFLAGS)
LIBS=$(SYSTEMLIBS) $(PD_LDFLAGS) $(GEM_LDFLAGS) $(PDGST_LDFLAGS) $(GST_LIBS)

EXT=pd_linux

SOURCE := $(wildcard pix_*.cpp)
OBJECTS := $(SOURCE:.cpp=.o)
TARGETS := $(SOURCE:.cpp=.$(EXT))

.PHONY: default all

# all builds gem-externals and plugins (gstreamer)

all: $(TARGETS)

$(TARGETS) : %.$(EXT) : %.o pdgstGem.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

pdgstGem.o $(OBJECTS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<


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
