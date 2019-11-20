#include "bonfire.h"
#include "uart.h"
#include "bonfire_gpio.h"
#include "mem_rw.h"


int main() {

int i=0;

  
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_EN,0xf);
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,0x9);
  wait(3000000); 
  setBaudRate(PLATFORM_BAUDRATE);
  
 //_setDivisor(0x340);
 // for(i=0;i<3;i++) {
  while(1) { 
    _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL, 1 << (i++ % 4 ) );  
    wait(1000000);
    if ((i % 4) == 0) {
      
      writestr("Bonfire \n\r");
    } 
  }   
 
 // }
  //writechar('\x1a'); // Simulation end marker
}
