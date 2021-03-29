## tlib: Multithreading library for Linux kernel

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

1. <b>One-one model</b>
	- One one model will use the clone system call to create a corresponding kernel thread for each user level thread. It is referred to as LWP.
	- LWPs share a same set of resources that is allocated to the process. These include address space, physical memory pages, file systems, open files and signal handlers. 
	- Extensive resource sharing is the reason these processes are called light-weight processes. 

2. <b>Many-one model</b>
	- There is a mapping for many user level threads to a single kernel thread. 
	- The library handles the multiplexing of the threads into a single kernel thread with aspects such as scheduling, managing the context of threads etc.



# One One Model

## Thread Creation
Thread creation lies on clone system call entirely for the one-one model. `clone()` system call with the right flags helps replicate the concept of lightweight process. 

##### Important flags related to the clone system call.
```js
- CLONE_VM 
	Shares Memory Space accross the process and the thread
- CLONE_FS
	Share the filesystem information between the parent and the child
- CLONE_FILES
	Shares the list of file descriptors between the parent and the child
- CLONE_PARENT_SETTID
	Before returning, the parent process sets the thread id at the location specified to clone
- CLONE_CHILD_CLEARTID
	After the thread terminates, the child will clear the thread id that was set at location previously by parent
- CLONE_THREAD
	Child thread is placed in the same thread group as the calling process 
- CLONE_SIGHAND
	Shares the list of signal handlers between the child and the parent process
- CLONE_SYSVSEM
	Share a single list of semaphore adjustment from System V
```
In one one model, each thread gets mapped to a single kernel thread and hence scheduling is handled by the kernel.
Creation of a one one thread should take a routine (function pointer) that the thread will execute, arguments to the function call and attibutes that the thread can be initialised with.
The `clone()` system call can be invoked on a per thread basis for creating a LWP in the kernel. The creation should make sure that the thread id returned by the clone system call is set in the thread that was passed as the argument to the call of create.
Since the threads have a seperate stack region, a stack needs to be allocated and should be passed as argument to the call to the `clone()` call. We need a stack of some size which is a mulitple of the page size of the system and an extra page which can be used for stack protection. This is the guard page that can be used to prevent stack overflow or memory leak issues. The base address of the memory aligned stack created using `posix_memalign()` and the protected page created via `mprotect()` is then passed to the `clone()` call.  

## Thread Attributes
The threads can have muliple attributes that define the creation and joining of the threads. Few of the attributes implemented are:
1. Stack Size - Lets the user set and get the size of the stack for the current thread
2. Guard Size - Lets the user set and get the size of the guard page of the stack that helps in stack protection.
3. Stack Address - Defines the stack region that the thread uses.

Note that the attributes object should be initialized before use and should be passed as an argument to the call of the `clone()` and these attributes should be set using the respective setter functions. A good programming practise is to free the attribute object by calling the attribute_destroy function call. 

## Thread Join

## Thread Exit

## Locking Mechanisms
1. Mutex
2. Spinlocks

## Signal Handling

## References
<a href="https://github.com/rxi/log.c">Logging Library</a>