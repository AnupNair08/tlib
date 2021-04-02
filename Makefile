CC := gcc
CFLAGS := -g -DLOG_USE_COLOR 
SUBDIRS := doxygen bin
SRC := src/*.c
OBJ := thread.o tattr.o dataStructs.o locks.o log.o manyone.o
TESTS := tests/*.c
BIN := bin/

.PHONY: init clean alltest tlib

init:
	@for dir in $(SUBDIRS); \
		do \
			if [ ! -d $$dir ]; \
			then \
				mkdir $$dir; \
			fi \
		done

# Compile all the binaries for the library
# Stores the binaries in the bin/ directory
tlib: $(SRC)
	$(CC) $(CFLAGS) -Isrc/ -c $(SRC) 
	@mv *.o $(BIN)

# Compile all binaries for the test suite
# All binaries and executable files get stored in bin/ directory
alltest: $(TESTS) $(SRC)
	$(CC) $(CFLAGS) -Isrc/ -c $(TESTS) $(SRC) 
	$(CC) unitTests.o $(OBJ) -o unitTests
	$(CC) lockTests.o $(OBJ) -o lockTests
	$(CC) matrix.o $(OBJ) -o matrix 
	$(CC) readers.o $(OBJ) -o readers
	$(CC) -g manyTests.o $(OBJ) -o manyTests
	$(CC) -g benchmark.o $(OBJ) -o benchmark

	@mv *.o unitTests lockTests matrix readers manyTests benchmark $(BIN)
	
	
clean:
	@rm $(BIN)*.o

docs:
	doxygen doxyconfig
