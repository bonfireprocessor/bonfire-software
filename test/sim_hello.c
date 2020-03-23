#include "bonfire.h"
#include "uart.h"
#include "bonfire_gpio.h"
#include "mem_rw.h"
#include "console.h"


uint64_t get_timer_value()
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


void printInfo()
{

#ifdef BONFIRE_CORE

  printk("UART Divisor: %d\n",
         getDivisor());
#else
  printk("MIMPID: %lx\nMISA: %lx\nUART Divisor: %d\n",
         read_csr(mimpid),read_csr(misa),
         getDivisor());
#endif 
}


int main() {

int i=0;

  
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_EN,0xf);
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,0x9);
  wait(3000000); 
  setBaudRate(PLATFORM_BAUDRATE);
  printInfo();
  
  //printk("ImpID: %x\n",get_impid());
  
  printk("SRAM base %x\n",SRAM_BASE);

  while(1) { 
    _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL, 1 << (i++ % 4 ) );  
    wait(1000000);
    if ((i % 4) == 0) {
  #ifndef BONFIRE_CORE    
      printk("Uptime: %d sec\n",(int)(get_timer_value()/SYSCLK));
  #else
      printk("run number %d\n",i);
  #endif    
  #ifdef SIM
    if (i>4) writechar(0x1a); // Terminate Simulation  
  #endif
    } 
  }   
 
 
}
