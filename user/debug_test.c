#include <stdio.h>
#include "console.h"

#include "riscv-gdb-stub.h"

void do_increment(volatile int *px)
{
  (*px)++;
}

#define BAUDRATE 115200

void main()
{
volatile int i=0;

  printk("Run with baudrate %d\n",BAUDRATE);
  gdb_setup_interface(BAUDRATE);
  gdb_initDebugger(1);
  gdb_breakpoint();





  while(1) {

    do_increment(&i);
    if ((i % 10000)==0 ) printk("*");
  }
}
