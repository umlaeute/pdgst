PdGst - GStreamer bindings for Pure Data
========================================

PdGst make GStreamer elements available inside a Pd-patch.
you can use it to create (and control) your GStreamer-pipelines in a visual
programming language.
there are also some bridge-objects, that allow you to move (audio, video/gem)
data between Pd and GStreamer.

prerequisites
-------------
so far, PdGst is developped on linux. it might or might not work on other
systems (like OSX or w32).

PdGst needs both GStreamer and Pure Data installed with all the development
headers. You might also want to have the development headers for Gem
installed.
on debian and friends (e.g. ubuntu) you might want to do something like the
following (as root):
    # aptitude install puredata-dev gem-dev
    # aptitude install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
you could also use `apt-get` instead of `aptitude`


build
-----
    $ make PD_INCLUDE=/usr/include/pd GEM_INCLUDE=/usr/include/Gem

install
-------
    for a system-wide installation run (as root, e.g. using `sudo`):
    # make install
    if you only want to install per-user, you can run
    $ make install pkglibdir=~/pd-externals


documentation
-------------
is sparse.
check out all the examples in tests/
there's also a paper on the background of PdGst which can be found at
  http://puredata.info/community/conventions/convention09/zmolnig.pdf
