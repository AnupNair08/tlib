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


alltest: tests/unitTests.c tests/lockTests.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c
	@$(CC) -Isrc/ -c tests/unitTests.c tests/lockTests.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c
	@$(CC) -g unitTests.o thread.o tattr.o dataStructs.o locks.o -o unitTests
	@$(CC) -g lockTests.o thread.o tattr.o dataStructs.o locks.o -o lockTests
	
	@./unitTests 
	@rm unitTests
	@./lockTests
	@rm lockTests

unittest: tests/unitTests.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c
	@$(CC) -Isrc/ -c tests/unitTests.c tests/lockTests.c src/thread.c src/tattr.c src/dataStructs.c src/locks.c
	@$(CC) -g unitTests.o thread.o tattr.o dataStructs.o locks.o -o unitTests
	
	@./unitTests 
	@rm unitTests
	
clean:
	@rm *.o

docs:
	doxygen doxyconfig
