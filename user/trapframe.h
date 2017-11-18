#ifndef __TRAPFRAME_H
#define __TRAPFRAME_H

typedef struct
{
  long gpr[32];
  long status;
  long epc;
  long badvaddr;
  long cause;
  long insn;
} trapframe_t;


#endif
