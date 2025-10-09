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


void printInfo(uint32_t level)
{

  uint32_t uart_control=_read_word((void*)UART_BASE+8);
  printk("UART Extended Control Reg: %lx\n",uart_control);
  printk("UART Status Reg: %lx\n",_read_word((void*)UART_BASE+4));
  if (level==1) {
    printk("Baudrate: %d\n",PLATFORM_BAUDRATE);
    printk("UART Divisor: %d\n",getDivisor());
    printk("MIMPID: %lx\n",read_csr(mimpid));
    printk("MISA: %lx\n",read_csr(misa));
  }
}

void led_out(uint32_t v)
{
  _write_word((void*)LED_BASE,v & 0xff);
}

void set_and_check_gpio(uint32_t offset ,uint32_t value)
{
  _write_word((void*)GPIO_BASE+offset,value);
  uint32_t r = _read_word((void*)GPIO_BASE+offset);
  printk("GPIO at offset %x wrote %x read %x\n",offset,value,r);
}

void set_and_check_led(uint32_t value)
{
  _write_word((void*)LED_BASE,value& 0xff);
  uint32_t r = _read_word((void*)LED_BASE);
  printk("LED wrote %x read %x\n",value & 0xff,r);
}


int main() {

int i=0;
uint32_t stage=1;

  led_out(stage++);
  setBaudRate(PLATFORM_BAUDRATE);
  set_and_check_led(0x55);
  // set_and_check_gpio(GPIO_OUTPUT_EN,0x55);
  // set_and_check_gpio(GPIO_OUTPUT_EN,0xff);
  // set_and_check_gpio(GPIO_OUTPUT_VAL,0x9);

  led_out(stage++);
  //uint32_t test=_read_word((void*)UART0_BASE+4); // Dummy Read to avoid spurious first read
  //led_out(test);
  
  #ifndef SIM
  wait(1000000);
  #endif
  led_out(stage++);
 
  write_console("\nHello from Bonfire Core!\n");
  led_out(stage++);
  printInfo(1);

  printk("SRAM base %x\n",SRAM_BASE);
  led_out(stage++);
  while(1) {
     #ifndef SIM
     wait(1000000);
     #endif
     uint32_t v =  1 << (i++ % 8 );
    _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,v);
     led_out(v);
    if ((i % 8) == 0) {
    #ifndef BONFIRE_CORE
        printk("Uptime: %d sec\n",(int)(get_timer_value()/SYSCLK));
    #else
        printk("run number %d\n",i);
        printInfo(0);
    #endif
    #ifdef SIM
      if (i>4) writechar(0x1a); // Terminate Simulation
    #endif
    }
  }


}
