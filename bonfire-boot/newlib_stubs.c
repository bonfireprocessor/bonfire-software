
#include <errno.h>
#include "console.h"

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