/* rt_futex.c -- Futex operations

   PROVIEW/R
   Contains functions that are heavily os-dependant.  
   
   Author: Robert Karlsson 21 Apr 2004 
   
   Description:
   This module provides an interface to futexes - "fast user level
   locking in Linux". This is achieved through the multiplexing
   system call sys_futex(). As implemented below this interface provides
   a synchronization mechanism that can be used both between threads
   in one process as well as between threads in different processes */

#if !defined(OS_LINUX)
# error "This file is valid only for OS_LINUX"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#define FUTEX_WAIT (0)
#define FUTEX_WAKE (1)
#define FUTEX_FD (2)
#define FUTEX_REQUEUE (3)


int 
futex_wait (
      int *futex, 
      int val
)
{
  int  ok;
  ok = syscall(SYS_futex, futex, FUTEX_WAIT, val, NULL);
  if (ok == -1) {
    return errno;
  }
  else {
    return ok;
  }
}

int 
futex_timed_wait (
      int *futex, 
      int val, 
      const struct timespec * timespec
)
{
  int  ok;
  ok = syscall(SYS_futex, futex, FUTEX_WAIT, val, timespec);
  if (ok == -1) {
    return errno;
  }
  else {
    return ok;
  }
}

int 
futex_wake (
      int *futex,
      int nr
)
{
  int  ok;
  ok = syscall(SYS_futex, futex, FUTEX_WAKE, nr, NULL);
  if (ok == -1) {
    return errno;
  }
  else {
    return ok;
  }
}
