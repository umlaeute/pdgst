all: src src/gem

.PHONY: src src/gem

src src/gem:
	make -C $@
