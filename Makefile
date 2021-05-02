# Flags and constants
CC := gcc
CFLAGS := -g -ggdb3
SUBDIRS := doxygen bin bin/ManyOne bin/OneOne
SRCMANYONE := src/ManyOne/*.c
SRCONEONE := src/OneOne/*.c
OBJMANYONE := thread.o tattr.o utils.o locks.o
OBJONEONE := thread.o tattr.o utils.o locks.o
TESTSMANYONE := src/ManyOne/tests/*.c
TESTSONEONE := src/OneOne/tests/*.c
BINMANYONE := bin/ManyOne/
BINONEONE := bin/OneOne/

.PHONY: init clean alltest tlib docs run

# Make initliazes all the needed directories for the project
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
tlib: $(SRCONEONE) $(SRCMANYONE)
	$(CC) $(CFLAGS) -c $(SRCONEONE) 
	@mv *.o $(BINONEONE)
	$(CC) $(CFLAGS) -c $(SRCMANYONE) 
	@mv *.o $(BINMANYONE)

# Compile all binaries for the test suite
# All binaries and executable files get stored in bin/ directory
alltest: $(TESTSMANYONE) $(TESTSONEONE) $(SRCMANYONE) $(SRCONEONE)
	@make init
	$(CC) $(CFLAGS) -c $(TESTSMANYONE) $(SRCMANYONE) 
	$(CC) unitTests.o $(OBJMANYONE) -o unitTests
	$(CC) lockTests.o $(OBJMANYONE) -o lockTests
	$(CC) robust.o $(OBJMANYONE) -o robust 
	$(CC) matrix.o $(OBJMANYONE) -o matrix 
	$(CC) readers.o $(OBJONEONE) -o readers 

	@mv *.o unitTests lockTests matrix robust readers $(BINMANYONE)
	$(CC) $(CFLAGS) -c $(TESTSONEONE) $(SRCONEONE) 
	$(CC) unitTests.o $(OBJONEONE) -o unitTests
	$(CC) lockTests.o $(OBJONEONE) -o lockTests
	$(CC) benchmark.o $(OBJONEONE) -o benchmark 
	$(CC) robust.o $(OBJONEONE) -o robust 
	$(CC) readers.o $(OBJONEONE) -o readers 
	@mv *.o unitTests robust lockTests benchmark readers $(BINONEONE)

run:
	make alltest
	./run.sh

# Run valgrind on the unit test to check for memory leaks	
check-leak:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes  --verbose \
	--log-file=leak-summary.txt  ./${BINONEONE}/unitTests

# Clean the compiled object files
clean:
	@rm $(BINMANYONE)*.o
	@rm $(BINONEONE)*.o

# Generate detailed documentation
docs:
	doxygen doxyconfig
