#include "bonfire.h"

#include <stdio.h>
#include<stdarg.h>


void platform_debug_printk(const char* s, ...)
{
va_list args;

    va_start (args, s);
    vprintf (s, args);
    va_end (args);
  
}

uint64_t platform_timer_sys_raw_read()
{
#if __riscv_xlen == 32
  while (1) {
    uint32_t hi = read_csr(mcycleh);
    uint32_t lo = read_csr(mcycle);
    if (hi == read_csr(mcycleh))
      return ((uint64_t)hi << 32) | lo;
  }
#else
  return read_csr(mcycle);
#endif

}

uint64_t platform_timer_read_sys( void )
{

    return platform_timer_sys_raw_read() * 1000000L  / (uint64_t)SYSCLK;
}
