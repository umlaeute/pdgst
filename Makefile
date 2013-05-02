default: all
## Pd library template version 1.0.14
# For instructions on how to use this template, see:
#  http://puredata.info/docs/developer/MakefileTemplate
LIBRARY_NAME = pdgst

# add your .c source files, one object per file, to the SOURCES
# variable, help files will be included automatically, and for GUI
# objects, the matching .tcl file too
SOURCES = pdgst.c \
	pix_gst2pix.cpp pix_pix2gst.cpp

SHARED_SOURCE =  \
	pdgst_base.c pdgst_capsfilter.c pdgst_element.c \
	gvalue.c loop.c pdgst.c properties.c \
	setup.c \
	pdgst_adc~.c pdgst_dac~.c

SHARED_HEADER = \
	include/pdgst/element.h \
	include/pdgst/properties.h \
	include/pdgst/pdgst.h


GSTGEM_SOURCES = pdgstGem.cpp
GSTGEM_HEADERS = pdgstGem.h pix_gst2pix.h pix_pix2gst.h

SHARED_LIB = libpdgst.$(SHARED_EXTENSION)
## shared lib for gst/gem is handled explicitely
# due to the nature of make, the explicit dependencies have to be defined
# after the implicit ones. scroll down to @GSTGEM_DEPENDENCIES@


# list all pd objects (i.e. myobject.pd) files here, and their helpfiles will
# be included automatically
#PDOBJECTS = mypdobject.pd

# example patches and related files, in the 'examples' subfolder
#EXAMPLES = bothtogether.pd

# manuals and related files, in the 'manual' subfolder
#MANUAL = manual.txt

# if you want to include any other files in the source and binary tarballs,
# list them here.  This can be anything from header files, test patches,
# documentation, etc.  README.md and LICENSE.txt are required and therefore
# automatically included
EXTRA_DIST = 

# unit tests and related files here, in the 'unittests' subfolder
UNITTESTS = 



#------------------------------------------------------------------------------#
#
# things you might need to edit if you are using other C libraries
#
#------------------------------------------------------------------------------#

GST_CFLAGS=$(shell pkg-config --cflags gstreamer-0.10)
GST_LIBS=$(shell pkg-config --libs gstreamer-0.10)
GST_PLUGIN_CFLAGS=-DPDGST_PLUGIN $(shell pkg-config --cflags gstreamer-plugins-base-0.10)
GST_PLUGIN_LIBS=-lgstapp-0.10 -lgstaudio-0.10 $(shell pkg-config --libs gstreamer-plugins-base-0.10)

ALL_CFLAGS = -I"$(PD_INCLUDE)" -I"$(GEM_INCLUDE)" -I"$(GEM_INCLUDE)/deprecated" -Iinclude/ -Isrc/ $(GST_CFLAGS) $(GST_PLUGIN_CFLAGS)
ALL_LDFLAGS =  
SHARED_LDFLAGS =
ALL_LIBS = $(GST_LIBS) $(GST_PLUGIN_LIBS)

## where to find the source-files
vpath %.h include
vpath %.c src
vpath %.c src/audio
vpath %.cpp src/gem
vpath %.h   src/gem

## where to find the source-files (dist)
vpath %.cpp src
vpath %.h   src
vpath %.h   src/pdgst

GSTGEM_OBJECTS=$(GSTGEM_SOURCES:%.cpp=%.o)
EXTERNALS=$(patsubst %.c,%,$(patsubst %.cpp,%,$(SOURCES)))
OFILES=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SHARED_SOURCE))) $(GSTGEM_OBJECTS) $(EXTERNALS:=.o)
EXTFILES=$(EXTERNALS:=.$(EXTENSION))


#------------------------------------------------------------------------------#
#
# you shouldn't need to edit anything below here, if we did it right :)
#
#------------------------------------------------------------------------------#

# these can be set from outside without (usually) breaking the build
CFLAGS = -Wall -W -g
LDFLAGS =
LIBS =

# get library version from meta file
LIBRARY_VERSION = $(shell sed -n 's|^\#X text [0-9][0-9]* [0-9][0-9]* VERSION \(.*\);|\1|p' $(LIBRARY_NAME)-meta.pd)

ALL_CFLAGS += -DPD -DVERSION='"$(LIBRARY_VERSION)"'

PD_INCLUDE = $(PD_PATH)/include/pd
GEM_INCLUDE = $(GEM_PATH)/include/Gem
# where to install the library, overridden below depending on platform
prefix = /usr/local
libdir = $(prefix)/lib
pkglibdir = $(libdir)/pd-externals
objectsdir = $(pkglibdir)

INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -p -m 644
INSTALL_DATA = $(INSTALL) -p -m 644
INSTALL_DIR     = $(INSTALL) -p -m 755 -d

ALLSOURCES := $(SOURCES) $(SOURCES_android) $(SOURCES_cygwin) $(SOURCES_macosx) \
	         $(SOURCES_iphoneos) $(SOURCES_linux) $(SOURCES_windows) \
           $(GSTGEM_SOURCES) $(GSTGEM_HEADERS) \
           $(SHARED_SOURCE)

DISTDIR=$(LIBRARY_NAME)-$(LIBRARY_VERSION)
ORIGDIR=pd-$(LIBRARY_NAME:~=)_$(LIBRARY_VERSION)

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  CPU := $(shell uname -p)
  ifeq ($(CPU),arm) # iPhone/iPod Touch
    SOURCES += $(SOURCES_iphoneos)
    EXTENSION = pd_darwin
    SHARED_EXTENSION = dylib
    OS = iphoneos
    PD_PATH = /Applications/Pd-extended.app/Contents/Resources
    IPHONE_BASE=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin
    CC=$(IPHONE_BASE)/gcc
    CPP=$(IPHONE_BASE)/cpp
    CXX=$(IPHONE_BASE)/g++
    ISYSROOT = -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk
    IPHONE_CFLAGS = -miphoneos-version-min=3.0 $(ISYSROOT) -arch armv6
    OPT_CFLAGS = -fast -funroll-loops -fomit-frame-pointer
    ALL_CFLAGS := $(IPHONE_CFLAGS) $(ALL_CFLAGS)
    ALL_LDFLAGS += -arch armv6 -bundle -undefined dynamic_lookup $(ISYSROOT)
    SHARED_LDFLAGS += -arch armv6 -dynamiclib -undefined dynamic_lookup $(ISYSROOT)
    ALL_LIBS += -lc $(LIBS_iphoneos)
    STRIP = strip -x
    DISTBINDIR=$(DISTDIR)-$(OS)
  else # Mac OS X
    SOURCES += $(SOURCES_macosx)
    EXTENSION = pd_darwin
    SHARED_EXTENSION = dylib
    OS = macosx
    PD_PATH = /Applications/Pd-extended.app/Contents/Resources
    OPT_CFLAGS = -ftree-vectorize -ftree-vectorizer-verbose=2 -fast
# build universal 32-bit on 10.4 and 32/64 on newer
    ifeq ($(shell uname -r | sed 's|\([0-9][0-9]*\)\.[0-9][0-9]*\.[0-9][0-9]*|\1|'), 8)
      FAT_FLAGS = -arch ppc -arch i386 -mmacosx-version-min=10.4
    else
      SOURCES += $(SOURCES_iphoneos)
# Starting with Xcode 4.0, the PowerPC compiler is not installed by default
      ifeq ($(wildcard /usr/llvm-gcc-4.2/libexec/gcc/powerpc*), )
        FAT_FLAGS = -arch i386 -arch x86_64 -mmacosx-version-min=10.5
      else
        FAT_FLAGS = -arch ppc -arch i386 -arch x86_64 -mmacosx-version-min=10.4
      endif
    endif
    ALL_CFLAGS += $(FAT_FLAGS) -fPIC -I/sw/include
    # if the 'pd' binary exists, check the linking against it to aid with stripping
    BUNDLE_LOADER = $(shell test ! -e $(PD_PATH)/bin/pd || echo -bundle_loader $(PD_PATH)/bin/pd)
    ALL_LDFLAGS += $(FAT_FLAGS) -headerpad_max_install_names -bundle $(BUNDLE_LOADER) \
	-undefined dynamic_lookup -L/sw/lib
    SHARED_LDFLAGS += $(FAT_FLAGS) -dynamiclib -undefined dynamic_lookup \
	-install_name @loader_path/$(SHARED_LIB) -compatibility_version 1 -current_version 1.0
    ALL_LIBS += -lc $(LIBS_macosx)
    STRIP = strip -x
    DISTBINDIR=$(DISTDIR)-$(OS)
# install into ~/Library/Pd on Mac OS X since /usr/local isn't used much
    pkglibdir=$(HOME)/Library/Pd
  endif
endif
# Tho Android uses Linux, we use this fake uname to provide an easy way to
# setup all this things needed to cross-compile for Android using the NDK
ifeq ($(UNAME),ANDROID)
  CPU := arm
  SOURCES += $(SOURCES_android)
  EXTENSION = so
  SHARED_EXTENSION = so
  OS = android
  PD_PATH = /usr
  NDK_BASE := /usr/local/android-ndk
  NDK_PLATFORM_LEVEL ?= 5
  NDK_ABI=arm
  NDK_SYSROOT=$(NDK_BASE)/platforms/android-$(NDK_PLATFORM_LEVEL)/arch-$(NDK_ABI)
  NDK_UNAME := $(shell uname -s | tr '[A-Z]' '[a-z]')
  NDK_COMPILER_VERSION=4.6
  NDK_TOOLCHAIN=$(wildcard \
	$(NDK_BASE)/toolchains/$(NDK_ABI)*-$(NDK_COMPILER_VERSION)/prebuilt/$(NDK_UNAME)-x86)
  CC := $(wildcard $(NDK_TOOLCHAIN)/bin/*-linux-android*-gcc) --sysroot=$(NDK_SYSROOT)
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  CFLAGS += 
  LDFLAGS += -rdynamic -shared
  SHARED_LDFLAGS += -Wl,-soname,$(SHARED_LIB) -shared
  LIBS += -lc $(LIBS_android)
  STRIP := $(wildcard $(NDK_TOOLCHAIN)/bin/$(NDK_ABI)-linux-android*-strip) \
	--strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq ($(UNAME),Linux)
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  SHARED_EXTENSION = so
  OS = linux
  PD_PATH = /usr
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -rdynamic -shared -fPIC -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
  SHARED_LDFLAGS += -Wl,-soname,$(SHARED_LIB) -shared
  ALL_LIBS += -lc $(LIBS_linux)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq ($(UNAME),GNU)
  # GNU/Hurd, should work like GNU/Linux for basically all externals
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  SHARED_EXTENSION = so
  OS = linux
  PD_PATH = /usr
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -rdynamic -shared -fPIC -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
  SHARED_LDFLAGS += -shared -Wl,-soname,$(SHARED_LIB)
  ALL_LIBS += -lc $(LIBS_linux)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq ($(UNAME),GNU/kFreeBSD)
  # Debian GNU/kFreeBSD, should work like GNU/Linux for basically all externals
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  SHARED_EXTENSION = so
  OS = linux
  PD_PATH = /usr
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -rdynamic -shared -fPIC -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
  SHARED_LDFLAGS += -shared -Wl,-soname,$(SHARED_LIB)
  ALL_LIBS += -lc $(LIBS_linux)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)-$(shell uname -m)
endif
ifeq (CYGWIN,$(findstring CYGWIN,$(UNAME)))
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_cygwin)
  EXTENSION = dll
  SHARED_EXTENSION = dll
  OS = cygwin
  PD_PATH = $(shell cygpath $$PROGRAMFILES)/pd
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += 
  ALL_LDFLAGS += -rdynamic -shared -L"$(PD_PATH)/src" -L"$(PD_PATH)/bin"
  SHARED_LDFLAGS += -shared -Wl,-soname,$(SHARED_LIB)
  ALL_LIBS += -lc -lpd $(LIBS_cygwin)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  CPU := $(shell uname -m)
  SOURCES += $(SOURCES_windows)
  EXTENSION = dll
  SHARED_EXTENSION = dll
  OS = windows
  PD_PATH = $(shell cd "$$PROGRAMFILES/pd" && pwd)
  # MinGW doesn't seem to include cc so force gcc
  CC=gcc
  OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -mms-bitfields
  ALL_LDFLAGS += -s -shared -Wl,--enable-auto-import
  SHARED_LDFLAGS += -shared
  ALL_LIBS += -L"$(PD_PATH)/src" -L"$(PD_PATH)/bin" -L"$(PD_PATH)/obj" \
	-lpd -lwsock32 -lkernel32 -luser32 -lgdi32 -liberty $(LIBS_windows)
  STRIP = strip --strip-unneeded -R .note -R .comment
  DISTBINDIR=$(DISTDIR)-$(OS)
endif

GEM_PATH=$(PD_PATH)

# in case somebody manually set the HELPPATCHES above
HELPPATCHES ?= $(EXTERNALS:=-help.pd) $(PDOBJECTS:.pd=-help.pd)

ALL_CFLAGS := $(ALL_CFLAGS) $(CFLAGS) $(OPT_CFLAGS)
ALL_LDFLAGS := $(LDFLAGS) $(ALL_LDFLAGS)
ALL_LIBS := $(LIBS) $(ALL_LIBS)

SHARED_SOURCE ?= $(wildcard lib$(LIBRARY_NAME).c)
SHARED_HEADER ?= $(shell test ! -e $(LIBRARY_NAME).h || echo $(LIBRARY_NAME).h)
SHARED_LIB ?= $(SHARED_SOURCE:.c=.$(SHARED_EXTENSION))
SHARED_TCL_LIB = $(wildcard lib$(LIBRARY_NAME).tcl)

.PHONY = install libdir_install install-doc install-examples install-manual install-unittests \
	clean distclean \
	dist dist-src dist-headers dist-help \
	etags etags-pd etags-src etags-tcl \
	$(LIBRARY_NAME)

all: $(EXTFILES) $(SHARED_LIB)

%.o: %.c
	$(CC) $(ALL_CFLAGS) -o "$@" -c "$<"
%.o: %.cpp
	$(CXX) $(ALL_CFLAGS) -o "$@" -c "$<"

%.$(EXTENSION): %.o $(SHARED_LIB)
	$(CXX) $(ALL_LDFLAGS) -o "$@" $^  $(ALL_LIBS)
	chmod a-x "$@"

## explicit dependency for GSt/Gem
# @GSTGEM_DEPENDENCIES@
pix_pix2gst.$(EXTENSION): $(GSTGEM_OBJECTS)
pix_gst2pix.$(EXTENSION): $(GSTGEM_OBJECTS)

$(SHARED_LIB): $(SHARED_SOURCE:.c=.o)
	$(CC) $(SHARED_LDFLAGS) -o $(SHARED_LIB) $(SHARED_SOURCE:.c=.o) $(ALL_LIBS)

install: libdir_install

# The meta and help files are explicitly installed to make sure they are
# actually there.  Those files are not optional, then need to be there.
libdir_install: $(EXTFILES) $(SHARED_LIB) install-doc install-examples install-manual install-unittests
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(LIBRARY_NAME)-meta.pd \
		$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(EXTFILES))" || (\
		$(INSTALL_PROGRAM) $(EXTFILES) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME) && \
		$(STRIP) $(addprefix $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/,$(EXTFILES)))
	test -z "$(strip $(SHARED_LIB))" || \
		$(INSTALL_DATA) $(SHARED_LIB) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(wildcard $(SOURCES:.c=.tcl)))" || \
		$(INSTALL_DATA) $(wildcard $(SOURCES:.c=.tcl)) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(PDOBJECTS))" || \
		$(INSTALL_DATA) $(PDOBJECTS) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(SHARED_TCL_LIB))" || \
		$(INSTALL_DATA) $(SHARED_TCL_LIB) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)

install-doc:
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	test -z "$(strip $(HELPPATCHES))" || \
		$(INSTALL_DATA) $(HELPPATCHES) \
			$(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) README.md $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/README.md
	$(INSTALL_DATA) LICENSE.txt $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/LICENSE.txt

install-examples:
	test -z "$(strip $(EXAMPLES))" || \
		$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/examples && \
		for file in $(EXAMPLES); do \
			$(INSTALL_DATA) examples/$$file $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/examples; \
		done

install-manual:
	test -z "$(strip $(MANUAL))" || \
		$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/manual && \
		for file in $(MANUAL); do \
			$(INSTALL_DATA) manual/$$file $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/manual; \
		done

install-unittests:
	test -z "$(strip $(UNITTESTS))" || \
		$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/unittests && \
		for file in $(UNITTESTS); do \
			$(INSTALL_DATA) unittests/$$file $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/unittests; \
		done

clean:
	-rm -f -- $(OFILES)
	-rm -f -- $(EXTFILES)
	-rm -f -- $(LIBRARY_NAME).o
	-rm -f -- $(LIBRARY_NAME).$(EXTENSION)
	-rm -f -- $(SHARED_LIB)

distclean: clean
	-rm -f -- $(DISTBINDIR).tar.gz
	-rm -rf -- $(DISTBINDIR)
	-rm -f -- $(DISTDIR).tar.gz
	-rm -rf -- $(DISTDIR)
	-rm -f -- $(ORIGDIR).tar.gz
	-rm -rf -- $(ORIGDIR)


$(DISTBINDIR):
	$(INSTALL_DIR) $(DISTBINDIR)

libdir-src: $(ALLSOURCES)
	$(INSTALL_DIR) $(DISTBINDIR)/src
	$(INSTALL_DATA) $^ $(DISTBINDIR)/src

libdir: all $(DISTBINDIR) libdir-src
	$(INSTALL_DATA) $(LIBRARY_NAME)-meta.pd  $(DISTBINDIR)
	$(INSTALL_DATA) $(HELPPATCHES) $(DISTBINDIR)
	test -z "$(strip $(EXTRA_DIST))" || \
		$(INSTALL_DATA) $(EXTRA_DIST)    $(DISTBINDIR)
#	tar --exclude-vcs -czpf $(DISTBINDIR).tar.gz $(DISTBINDIR)

$(DISTDIR):
	$(INSTALL_DIR) $(DISTDIR)

$(ORIGDIR):
	$(INSTALL_DIR) $(ORIGDIR)

dist-headers: $(SHARED_HEADER)
	@echo $^
	$(INSTALL_DIR) $(DISTDIR)/src/pdgst && \
	$(INSTALL_DATA) $^ \
	  $(DISTDIR)/src/pdgst

dist-src: $(ALLSOURCES)
	test -z "$^" || \
		$(INSTALL_DIR) $(DISTDIR)/src && \
		$(INSTALL_DATA) $^ \
			$(DISTDIR)/src

dist-help: $(HELPPATCHES)
	test -z "$^" || \
		$(INSTALL_DATA) $^ \
			$(DISTDIR)

dist: $(DISTDIR) dist-src dist-headers dist-help
	$(INSTALL_DATA) Makefile  $(DISTDIR)
	$(INSTALL_DATA) README.md $(DISTDIR)
	$(INSTALL_DATA) LICENSE.txt $(DISTDIR)
	$(INSTALL_DATA) $(LIBRARY_NAME)-meta.pd  $(DISTDIR)
	test -z "$(strip $(wildcard $(ALLSOURCES:.c=.tcl)))" || \
		$(INSTALL_DATA) $(wildcard $(ALLSOURCES:.c=.tcl))  $(DISTDIR)
	test -z "$(strip $(wildcard $(LIBRARY_NAME).c))" || \
		$(INSTALL_DATA) $(LIBRARY_NAME).c  $(DISTDIR)
	test -z "$(strip $(SHARED_TCL_LIB))" || \
		$(INSTALL_DATA) $(SHARED_TCL_LIB)  $(DISTDIR)
	test -z "$(strip $(PDOBJECTS))" || \
		$(INSTALL_DATA) $(PDOBJECTS)  $(DISTDIR)
	test -z "$(strip $(EXTRA_DIST))" || \
		$(INSTALL_DATA) $(EXTRA_DIST)    $(DISTDIR)
	test -z "$(strip $(EXAMPLES))" || \
		$(INSTALL_DIR) $(DISTDIR)/examples && \
		for file in $(EXAMPLES); do \
			$(INSTALL_DATA) examples/$$file $(DISTDIR)/examples; \
		done
	test -z "$(strip $(MANUAL))" || \
		$(INSTALL_DIR) $(DISTDIR)/manual && \
		for file in $(MANUAL); do \
			$(INSTALL_DATA) manual/$$file $(DISTDIR)/manual; \
		done
	test -z "$(strip $(UNITTESTS))" || \
		$(INSTALL_DIR) $(DISTDIR)/unittests && \
		for file in $(UNITTESTS); do \
			$(INSTALL_DATA) unittests/$$file $(DISTDIR)/unittests; \
		done
	tar --exclude-vcs -czpf $(DISTDIR).tar.gz $(DISTDIR)

# make a Debian source package
dpkg-source:
	debclean
	make distclean dist
	mv $(DISTDIR) $(ORIGDIR)
	tar --exclude-vcs -czpf ../$(ORIGDIR).orig.tar.gz $(ORIGDIR)
	rm -f -- $(DISTDIR).tar.gz
	rm -rf -- $(DISTDIR) $(ORIGDIR)
	cd .. && dpkg-source -b $(LIBRARY_NAME)

etags: TAGS

etags-pd: $(wildcard $(PD_INCLUDE)/*.h)
	etags $^
etags-source: $(ALLSOURCES) $(SHARED_HEADER)
	etags -a *.h $^
etags-tcl:
	etags -a --language=none --regex="/proc[ \t]+\([^ \t]+\)/\1/" *.tcl

TAGS: etags-pd etags-source etags-tcl

showsetup:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIBS: $(LIBS)"
	@echo "ALL_CFLAGS: $(ALL_CFLAGS)"
	@echo "ALL_LDFLAGS: $(ALL_LDFLAGS)"
	@echo "ALL_LIBS: $(ALL_LIBS)"
	@echo "PD_INCLUDE: $(PD_INCLUDE)"
	@echo "PD_PATH: $(PD_PATH)"
	@echo "GEM_INCLUDE: $(GEM_INCLUDE)"
	@echo "GEM_PATH: $(GEM_PATH)"
	@echo "objectsdir: $(objectsdir)"
	@echo "LIBRARY_NAME: $(LIBRARY_NAME)"
	@echo "LIBRARY_VERSION: $(LIBRARY_VERSION)"
	@echo "SOURCES: $(SOURCES)"
	@echo "SHARED_HEADER: $(SHARED_HEADER)"
	@echo "SHARED_SOURCE: $(SHARED_SOURCE)"
	@echo "SHARED_LIB: $(SHARED_LIB)"
	@echo "SHARED_TCL_LIB: $(SHARED_TCL_LIB)"
	@echo "OFILES: $(OFILES)"
	@echo "EXTFILES: $(EXTFILES)"
	@echo "PDOBJECTS: $(PDOBJECTS)"
	@echo "ALLSOURCES: $(ALLSOURCES)"
	@echo "ALLSOURCES TCL: $(wildcard $(ALLSOURCES:.c=.tcl))"
	@echo "UNAME: $(UNAME)"
	@echo "CPU: $(CPU)"
	@echo "pkglibdir: $(pkglibdir)"
	@echo "DISTDIR: $(DISTDIR)"
	@echo "ORIGDIR: $(ORIGDIR)"
	@echo "NDK_TOOLCHAIN: $(NDK_TOOLCHAIN)"
	@echo "NDK_BASE: $(NDK_BASE)"
	@echo "NDK_SYSROOT: $(NDK_SYSROOT)"
