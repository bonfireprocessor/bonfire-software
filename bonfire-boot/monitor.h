#ifndef __MONITOR_H
#define __MONITOR_H

#include <stdint.h>



extern uint8_t __loadbase;  // Symbol defined in Linker script
extern uint8_t  __user_stack; // symbol defined by linker
extern void _start();

#define MY_BASE ((void*)&_start)
#define LOAD_BASE ((void*)&__loadbase)
#define LOAD_SIZE  ((intptr_t)MY_BASE -  (intptr_t)LOAD_BASE  )
#define USER_STACK ((intptr_t)&__user_stack & 0x0fffffff0)

typedef struct
{
  long gpr[32];
  long status;
  long epc;
  long badvaddr;
  long cause;
  long insn;
} trapframe_t;


void do_break(uint32_t arg0,...);

void start_user(uint32_t pc,uint32_t sp);

long sys_time(long* loc); // implemented in syscall.c

// RAM independant delay loop which takes 6 clocks/count
void delay_loop(uint32_t count);

#define LOOP_TIME (CLK_PERIOD * 6)

void test_dcache(int n);
void print_cache_size();
uint64_t platform_timer_read_sys( void );
void flush_dache();



int mon_main();
void shell();

//#define NO_FLASH
#define NO_SYSCALL
#define NO_XMODEM
#define NO_DRAMTEST

#ifdef PLATFORM_BAUDRATE
  #define BAUDRATE PLATFORM_BAUDRATE
#else   
    #define BAUDRATE 500000L
#endif

#define GDBSTUB 1

#endif
