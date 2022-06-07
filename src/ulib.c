#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#include "param.h"  // for NPROC
#define PGSIZE 4096

// these arrays are used to store the pid&pointer pairs, in order to 
// correctly free the allocated memory.
void* p_ptr[NPROC];   // pointer to the originally allocated memory
int pid[NPROC] = {0};  // pids for child threads
int use[NPROC] = {0};  // flag for indicating whether the pair is valid, default is invalid

// X86 fetch_and_add routine
static inline int fetch_and_add(int* variable, int value)
{
    __asm__ volatile("lock; xaddl %0, %1"
      : "+r" (value), "+m" (*variable) // input + output
      : // No input-only
      : "memory"
    );
    return value;
}

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

int
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
  void *stack, *p = malloc(PGSIZE * 2);
  int retVal = 0;

  // Check malloc return value
  if(p == 0){
    return -1;
  }

  // Force page size alignment, inspired from the provided tests
  if((uint)p % PGSIZE)
    stack = p + (PGSIZE - (uint)p % PGSIZE);
  else
    stack = p;

  // Create new thread
  retVal = clone(start_routine, arg1, arg2, stack);
  if(retVal <= 0){  // Fail to create
    free(p);
    return -1;
  }

  // store the pid&pointer pair into the data structures
  for(int i = 0; i < NPROC; i++){
    if(use[i] == 0){
      pid[i] = retVal;
      p_ptr[i] = p;
      use[i] = 1;  // flag to indicate this pair is valid
      break;
    }
  }

  return retVal;
}

int
thread_join()
{
  void* stack;
  void* p;
  int retVal;

  retVal = join(&stack);
  if(retVal <= 0){
    return -1;
  }

  // find the pid&pointer pair in the data structures
  for(int i = 0; i < NPROC; i++){
    if(use[i] == 1 && pid[i] == retVal){
      p = p_ptr[i];
      free(p);
      use[i] = 0;  // flag to indicate this pair is invalid
      break;
    }
  }

  return retVal;
}

void
lock_init(lock_t* mylock)
{
  mylock->ticket = 0;
  mylock->turn = 0;
}

void
lock_acquire(lock_t* mylock)
{
  int myturn = fetch_and_add(&mylock->ticket, 1);
  while(fetch_and_add(&mylock->turn, 0) != myturn );  // spin if not my turn
}

void
lock_release(lock_t* mylock)
{
  fetch_and_add(&mylock->turn, 1);
}
