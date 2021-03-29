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

test: src/caller.c src/thread.c src/dataStructs.c
	$(CC) -c src/caller.c src/thread.c src/tattr.c src/dataStructs.c
	$(CC) caller.o thread.o tattr.o dataStructs.o -o tout


alltest: tests/unitTests.c tests/lockTests.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c src/log.c
	@$(CC) -DLOG_USE_COLOR -Isrc/ -c tests/unitTests.c tests/lockTests.c src/thread.c src/tattr.c src/log.c  src/dataStructs.c src/locks.c 
	@$(CC) -g unitTests.o thread.o tattr.o dataStructs.o locks.o log.o -o unitTests
	@$(CC) -g lockTests.o thread.o tattr.o dataStructs.o locks.o log.o -o lockTests
	
	@./unitTests 
	@rm unitTests
	@./lockTests
	@rm lockTests

unittest: tests/unitTests.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c src/log.c tests/matrix.c
	@$(CC) -DLOG_USE_COLOR -Isrc/ -c tests/unitTests.c tests/lockTests.c tests/matrix.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c src/log.c
	@$(CC) -g unitTests.o thread.o tattr.o dataStructs.o locks.o log.o -o  unitTests
	@$(CC) -g matrix.o thread.o tattr.o dataStructs.o locks.o log.o -o  matrix

	# ./matrix
	# @rm unitTests
	@./unitTests 

	
readers: src/thread.c src/tattr.c src/dataStructs.c src/locks.c src/log.c tests/readers.c
	@$(CC) -DLOG_USE_COLOR -Isrc/ -c tests/readers.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c src/log.c
	@$(CC) -g thread.o tattr.o dataStructs.o readers.o locks.o log.o -o  readers

clean:
	@rm *.o

docs:
	doxygen doxyconfig
