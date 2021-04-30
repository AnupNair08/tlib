<div style = "text-align:center">
	<h1>tlib</h1>
	<h4>A multithreading library for Linux</h4> 
</div>

## Contents

- [About](#about)
- [Building](#building)
- [Mapping models](#mapping-models)
- [Available APIs](#available-apis)
- [Usage](#usage)
- [Running Tests](#running-tests)
- [References](#references)

### About

`tlib` is a program independent multithreading library that lets programs control multiple flows of work that run in an interleaved manner and supports user level threads in either a `One-One` mapping model or a `Many-One` mapping model. `tlib` is available for Unix-like POSIX conformant operating systems.

### Building

```
Run make in the root directory
To compile the binaries run
make tlib

To compile the test suite
make alltest

To run the tests
./run.sh
```

### Mapping models

A mapping model refers to the way in which a thread created by the user maps to the kernel level thread. There are three main types of which the two implemented in `tlib` are :

1. <b>One-one model</b>

   - One one model will use the `clone` system call to create a corresponding kernel thread for each user level thread. It is referred to as LWP.
   - LWPs share a same set of resources that are allocated to the process. These include address space, physical memory pages, file systems, open files and signal handlers.
   - Extensive resource sharing is the reason these processes are called light-weight processes.
   - One-one threading model is highly used in OS like Linux and Windows.

2. <b>Many-one model</b>

   - There is a mapping for many user level threads onto a single kernel thread.
   - The library handles the multiplexing of the threads into a single kernel thread with aspects such as scheduling, managing the context of threads etc.
   - The scheduling and context switching are entirely handled by the library and the kernel sees a single flow of execution.

   ```
                     User space                                            User space
        ┌─────────────────────────────────┐                   ┌─────────────────────────────────┐
        │                                 │                   │                                 │
        │                                 │                   │                                 │
        │                                 │                   │     ┌─┐  ┌─┐      ┌─┐ ┌─┐       │
        │                                 │                   │     └┼┘  └┼┘      └┼┘ └┼┘       │
        │                                 │                   │      │    │        │   │        │
        │     ┌─┐  ┌─┐      ┌─┐ ┌─┐       │                   │      │    │      ┌─┘   │        │
        │     └┼┘  └┼┘      └┼┘ └┼┘       │                   │      └───▲▼──────▼─◄───┘        │
        │      │    │        │   │        │                   │          │   map   │            │
        │      │    │        │   │        │                   │          │         │            │
        └──────┼────┼────────┼───┼────────┘                   └──────────┴────┬────┴────────────┘
               │    │        │   │                                            │
               │    │        │   │                                            │
        ┌──────┼────┼────────┼───┼────────┐                   ┌───────────────┼─────────────────┐
        │      │    │        │   │        │                   │               │                 │
        │      │    │        │   │        │                   │               │                 │
        │      │    │        │   │        │                   │               │                 │
        │      │    │       ┌┼┐ ┌┼┐       │                   │               │                 │
        │     ┌┼┐  ┌┼┐      └─┘ └─┘       │                   │              ┌┼┐                │
        │     └─┘  └─┘                    │                   │              └─┘                │
        │                                 │                   │                                 │
        │                                 │                   │                                 │
        │                                 │                   │                                 │
        └─────────────────────────────────┘                   └─────────────────────────────────┘
                    Kernel space                                          Kernel space


              One One Threading model                                Many One Threading model

   ```

## Available APIs

- `tlib` provides two mapping models and the desired model can be chosen without changing any API calls. The implementation remains abstract to the user programs.
- Each of the two models provides the following set of API calls:

1. <b>Thread APIs</b>

   1. <b>Thread Creation</b>

   | Function          | Description                                |
   | ----------------- | ------------------------------------------ |
   | `thread_create()` | Creates a new thread                       |
   | `thread_join()`   | Waits for a callee thread to be terminated |
   | `thread_kill()`   | Send a signal to a specific thread         |
   | `thread_exit()`   | Exit the thread routine                    |

   2. <b>Thread Attribute handling</b>

   | Function                 | Description                                         |
   | ------------------------ | --------------------------------------------------- |
   | `thread_attr_init()`     | Initialize an attribute object                      |
   | `thread_attr_destroy()`  | Destroy an attribute object                         |
   | `thread_attr_setStack()` | Set a user defined stack size                       |
   | `thread_attr_getStack()` | Get the size of the current thread stack            |
   | `thread_attr_setGuard()` | Set a user defined guard page size                  |
   | `thread_attr_getGuard()` | Get the size of the current thread stack guard page |

2. <b>Synchronization Primitves APIs</b>

   1. <b>Spin Lock</b>

   | Function         | Description                         |
   | ---------------- | ----------------------------------- |
   | `spin_init()`    | Initialize a spinlock object        |
   | `spin_acquire()` | Acquire a spinlock                  |
   | `spin_release()` | Release a spinlock                  |
   | `spin_trylock()` | Check if a spinlock can be acquired |

   2. <b>Mutex Lock</b>

   | Function          | Description                      |
   | ----------------- | -------------------------------- |
   | `mutex_init()`    | Initialize a mutex object        |
   | `mutex_acquire()` | Acquire a mutex                  |
   | `mutex_release()` | Release a mutex                  |
   | `mutex_trylock()` | Check if a mutex can be acquired |

## Usage

To use tlib in your programs, do the following:

```c
// Use on of the macros to use the desired mapping
#define ONE_ONE
// #define MANY_ONE
#include <stdio.h>

#ifdef ONE_ONE
   #include "src/OneOne/thread.h"
#else
   #include "src/ManyOne/thread.h"
#endif

int global_var = 0;
void func1(){
   printf("In thread routine 1");
   global_var++;
   return;
}

void func2(){
   printf("In thread routine 2");
   global_var++;
   return;
}

int main(){
   thread t1,t2;
   thread_create(&t1, NULL, func1, NULL);
   thread_create(&t2, NULL, func2, NULL);
   thread_join(t1,NULL);
   thread_join(t2,NULL);
   return 0;
}

```

## Running Tests

The library comes with an extensive test suite for checking the implementation and testing the performance of the library. Each implementation has a set of unit tests that check the correctness of the APIs. There is a test for checking the synchronization primitves and a classic program of readers writers to check the working of synchronization primitives namely mutex and spinlock. The test suite also includes a robust testing program that checks for the error handling and incorrect input cases. Finally there is a benchmark program which is a matrix multiplication program in the single and multi-threaded environments to compare the performance of using a threading library.

To run the tests

```
Run the following in the root directory of the project

# Compile and auto run
make run

# Compile all binaries
make alltest
# Start the test suite
./run.sh


# Check for memory leaks
make check-leak

```

The shell script will test all the functionalities mentioned in the test suite above. In addition, the memory leak checker uses valgrind to look at potential memory leaks in the test code.

## References

- [Pthread Programming Ch.6](https://maxim.int.ru/bookshelf/PthreadsProgram/htm/r_47.html)
- [Introduction to pthreads](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html)
- [Fundamentals of Multithreading](http://malgenomeproject.org/os2018fall/04_thread_b.pdf)
- [POSIX Linux Thread Library](https://www.cs.utexas.edu/~witchel/372/lectures/POSIX_Linux_Threading.pdf)
- [Implementing Threads](http://www.it.uu.se/education/course/homepage/os/vt18/module-4/implementing-threads/#kernel-level-threads)
- [POSIX Threads API](https://hpc-tutorials.llnl.gov/posix/)
- [Coroutines](http://www.csl.mtu.edu/cs4411.ck/common/Coroutines.pdf)
