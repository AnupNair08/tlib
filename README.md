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
1. <b>Mutex</b>
	Threads which try to access a critical section are made to acquire a lock in their entry sections. Mutex lock once acquired by a thread leave all other threads trying to acquire the same lock in a sleeping state. This ensures that only one thread has a lock when the critical section is being accessed. Further, once the thread is done with the lock, it releases the lock and all the waiting threads are woken up. The threads then contest for the locks once again. This sleep wait mechanism has been implemented with the help of `futex` system call and guarantees that operations for waiting on a lock are atomic in nature.

2. <b>Spinlocks</b>
	Spinlocks are locking mechansims wherein the waiting threads do not sleep, instead they do a busy waiting for the lock, trying to see if the lock is available in successive CPU cycles. The entry section of the code checks if the lock is already acquired by some other thread and if it is then the thread goes in a busy wait loop. Once the lock has been released, the thread can then acquire the lock during one of its cycles.

## Signal Handling

tlib uses two types of signal dispositions.
- Process wide signals: SIGINT, SIGCONT, SIGSTOP
	These signals are sent to all the threads running as a part of the process. The action will be asynchronous and hence all the threads will be sent the respective signal.
- Thread specific signals: SIGSEGV, SIGTERM, SIGABRT, SIGFPE
	These signals are delivered to only specific signals and there are custom handlers to handle the delivery of the signals. These signals are synchronous hence they wont have affect on any other threads that run in the process group.

## Scheduling policies

The One One implementation of tlib uses a System Contention Scope for scheduling whereas the Many One implementation uses the Process Contention Scope for scheduling. The Process Context Scope is a simple Round Robin based scheduling on either a preemptive or a non preemptive basis. The processes yeild to a CPU when they exit or move to a waiting queue. Also the processes can be moved to the ready queue when the timer interrupt occurs.


## References
<a href="https://github.com/rxi/log.c">Logging Library</a>