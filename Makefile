all: src src/gem

.PHONY: src src/gem

## the build system is assuming that your filesystem layout is like that:
### .../pd
### .../externals/Gem
### .../externals/pdgst
## (like the Pd-SVN layout)
##
## if your layout differs, use PD_SRC and GEM_SRC, like:
# $ make PD_SRC=/usr/include/pd GEM_SRC=/usr/include/Gem

src src/gem:
	make -C $@
