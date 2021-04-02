CC := gcc
CFLAGS := -g -DLOG_USE_COLOR 
SUBDIRS := doxygen bin bin/ManyOne bin/OneOne
SRCMANYONE := src/ManyOne/*.c
SRCONEONE := src/OneOne/*.c
OBJMANYONE := thread.o tattr.o dataStructs.o locks.o log.o
OBJONEONE := thread.o tattr.o dataStructs.o locks.o log.o
TESTSMANYONE := src/ManyOne/tests/*.c
TESTSONEONE := src/OneOne/tests/*.c
BINMANYONE := bin/ManyOne/
BINONEONE := bin/OneOne/

.PHONY: init clean alltest tlib docs

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
	$(CC) $(CFLAGS) -c $(TESTSMANYONE) $(SRCMANYONE) 
	$(CC) -g manyTests.o $(OBJMANYONE) -o manyTests
	$(CC) matrix.o $(OBJONEONE) -o matrix 
	@mv *.o manyTests matrix $(BINMANYONE)
	$(CC) $(CFLAGS) -c $(TESTSONEONE) $(SRCONEONE) 
	$(CC) unitTests.o $(OBJONEONE) -o unitTests
	$(CC) lockTests.o $(OBJONEONE) -o lockTests
	$(CC) matrix.o $(OBJONEONE) -o matrix 
	$(CC) readers.o $(OBJONEONE) -o readers
	@mv *.o unitTests lockTests matrix readers $(BINONEONE)
	
	
clean:
	@rm $(BINMANYONE)*.o
	@rm $(BINONEONE)*.o

docs:
	doxygen doxyconfig
