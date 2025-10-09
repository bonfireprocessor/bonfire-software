#include <stdint.h>
#include "platform.h"
#include "mem_rw.h"
// Test program intented for work on real hardware
//TH 03.10.2025: Support for 8  up to LEDs

//volatile uint8_t *gpioadr=(uint8_t *)GPIO_BASE;

int main(int argc,char ** argv) {

   uint32_t counter =0;

   _write_word((void*)GPIO_BASE,counter); // Use I/O Reg as Counter variable

    while(1) {
         counter=_read_word((void*)GPIO_BASE);
        _write_word((void*)LED_BASE, (counter++ >> 20) & 0xff); 
        _write_word((void*)GPIO_BASE,counter);       
   }

};
