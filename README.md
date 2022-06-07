# xv6-Kernel-threads
adding real kernel threads to xv6

## New system calls
1. clone int clone(void(*fcn)(void *, void *), void *arg1, void *arg2, void *stack) This call creates a new kernel thread which shares the calling process's address space.  File descriptors are copied as in fork(). The new process uses stack as its user stack, which is passed two arguments (arg1 and arg2) and uses a fake return PC (0xffffffff); a proper thread will simply call exit() when it is done (and not return). The stack should be one page in size and page-aligned. The new thread starts executing at the address specified by fcn. As with fork(), the PID of the new thread is returned to the parent (for simplicity, threads each have their own process ID).
2. join int join(void ** stack) The other new system call is int join(void ** stack). This call waits for a child thread that shares the address space with the calling process to exit. It returns the PID of waited-for child or -1 if none. The location of the child's user stack is copied into the argument stack (which can then be freed).

## thread library
1. thread_create: int thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2) This routine should call malloc() to create a new user stack, use clone() to create the child thread and get it running. start_routine and the 2 arguments are just the function pointer and arguments to be passed to clone.
2. thread_join: int thread_join() An int thread_join() call should also be created, which calls the underlying join() system call, frees the user stack, and then returns. It returns the waited-for PID (when successful), -1 otherwise.
3. ticket lock: Your thread library should also have a simple ticket lock (read this book chapterLinks to an external site. for more information on this). There should be a type lock_t that one uses to declare a lock, and two routines void lock_acquire(lock_t *) and void lock_release(lock_t *), which acquire and release the lock.
