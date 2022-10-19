
#include <errno.h>
#include "console.h"
#include "uart.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#undef errno
extern int  errno;




void *
_sbrk (int nbytes)
{
  /* Symbol defined by linker map */
  extern int  end;              /* start of free memory (as symbol) */
  extern int  _endofheap;

  /* Value set by crt0.S */
  static void *stack =(void*)&_endofheap;           /* end of free memory */

  /* The statically held previous end of the heap, with its initialization. */
  static void *heap_ptr = (void *)&end;         /* Previous end */

  //printk("sbrk called with %ld heap_ptr: %lx stack: %lx\n",nbytes,heap_ptr,stack);

  if ((stack - (heap_ptr + nbytes)) > 0 )
    {
      void *base  = heap_ptr;
      heap_ptr   += nbytes;
                
      return  base;
    }
  else
    {
      errno = ENOMEM;
      return  (void *) -1;
    }
}       /* _sbrk () */



int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

void __wrap__exit (int status)
{
	printk("_exit called with code %lx\n",status);
	while (1) {}		/* Make sure we hang here */
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = readchar();
	}

return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
	  for(int i=0;i<len;i++) writechar(*ptr++);
    return len;
}

int _close(int file)
{
	return -1;
}


int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _open(char *path, int flags, ...)
{
	/* Pretend like we always fail */
	return -1;
}

int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int _times(struct tms *buf)
{
	return -1;
}

int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}