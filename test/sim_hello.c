#include "bonfire.h"
#include "uart.h"
#include "bonfire_gpio.h"
#include "mem_rw.h"
#include "console.h"

#ifdef SIM
#pragma message "Compiling for Simulator"
#endif

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

  printk("UART Divisor: %d\n",getDivisor());

#ifndef BONFIRE_CORE
  printk("MIMPID: %lx\n",read_csr(mimpid));
  printk("MISA: %lx\n",read_csr(misa));
#endif

}

void led_out(uint32_t v)
{
  _write_word((void*)LED_BASE,v & 0xff);
}

int main() {

int i=0;
uint32_t stage=1;

  led_out(stage++);
  led_out(stage++);
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_EN,0xff);
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,0x9);
  //wait(3000000);
  led_out(stage++);
  uint32_t test=_read_word((void*)UART0_BASE+4); // Dummy Read to avoid spurious first read
  led_out(test);
  setBaudRate(PLATFORM_BAUDRATE);
  led_out(stage++);
  printInfo();

  printk("SRAM base %x\n",SRAM_BASE);
  led_out(stage++);
  while(1) {
     #ifndef SIM
     wait(1000000);
     #endif
     uint32_t v =  1 << (i++ % 4 );
    _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,v);
    #ifdef LED_BASE
      _write_word((void*)LED_BASE,v);
    #endif
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
