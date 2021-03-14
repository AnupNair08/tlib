CC=gcc
CFLAGS= 
SUBDIRS = doxygen

.PHONY: init

init:
	@for dir in $(SUBDIRS); \
		do \
			if [ ! -d $$dir ]; \
			then \
				mkdir $$dir; \
			fi \
		done

test: src/caller.c src/thread.c 
	$(CC) -c src/caller.c src/thread.c src/tattr.c
	$(CC) caller.o thread.o tattr.o -o tout


runtest: tests/create.c src/thread.c src/tattr.c
	@$(CC) -Isrc/ -c tests/create.c src/thread.c src/tattr.c
	@$(CC) -g create.o thread.o tattr.o -o create
	@./create
	@rm create

clean:
	@rm *.o

docs:
	doxygen doxyconfig
