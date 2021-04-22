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

1. <b>Thread related APIs</b>

   1. Thread Creation
      1. `thread_create()`
      2. `thread_join()`
      3. `thread_kill()`
      4. `thread_exit()`
   2. Thread Attribute handling
      1. `thread_attr_init()`
      2. `thread_attr_destroy()`
      3. `thread_attr_getStack()`
      4. `thread_attr_setStack()`
      5. `thread_attr_getGuard()`
      6. `thread_attr_setGuard()`

2. <b>Synchronization Primitves related APIs</b>
   1. Spin Lock
      1. `spin_init()`
      2. `spin_acquire()`
      3. `spin_release()`
      4. `spin_trylock()`
   2. Mutex Lock
      1. `mutex_init()`
      2. `mutex_acquire()`
      3. `mutex_release()`
      4. `mutex_trylock()`

## References

- [Pthread Programming Ch.6](https://maxim.int.ru/bookshelf/PthreadsProgram/htm/r_47.html)
