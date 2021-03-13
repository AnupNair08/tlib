### tlib: Multithreading library for Linux kernel

#### Mapping models

1. One-one model
2. Many-one model

One one model will use the clone system call to create a corresponding kernel thread for each user level thread. It is referred to as LWP.
LWPs share a same set of resources that is allocated to the process. These include address space, physical memory pages, file systems, open files and signal handlers. Extensive resource sharing is the reason these processes are called light-weight processes. 



Important flags related to the clone system call.
```js
     CLONE_VM, CLONE_FS, CLONE_FILES
	These flags select semantics with shared address space and
	file descriptors according to what POSIX requires.
     CLONE_SIGNAL
	This flag selects the POSIX signal semantics.
     CLONE_SETTLS
	The sixth parameter to CLONE determines the TLS area for the
	new thread.
     CLONE_PARENT_SETTID
	The kernels writes the thread ID of the newly created thread
	into the location pointed to by the fifth parameters to CLONE.
	Note that it would be semantically equivalent to use
	CLONE_CHILD_SETTID but it is be more expensive in the kernel.
     CLONE_CHILD_CLEARTID
	The kernels clears the thread ID of a thread that has called
	sys_exit() in the location pointed to by the seventh parameter
	to CLONE.
     CLONE_DETACHED
	No signal is generated if the thread exists and it is
	automatically reaped.
	CLONE_THREAD
	Makes the thread exist in the same process group as that of the main thread. 
	This makes sure that the only parent is the shell that forked the process and all other threads are part of this process group.
	
```