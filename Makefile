CC=gcc
CFLAGS=
SUBDIRS = doxygen

.PHONY: init

init:
	for dir in $(SUBDIRS); \
		do \
			mkdir $$dir; \
		done
	doxygen doxyconfig