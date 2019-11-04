#include "wildfire.h"
#include "uart.h"
#include "bonfire_gpio.h"

inline void _write_word(void* address,uint32_t value)
{
  *(( volatile uint32_t* )( address ))=value;
}

inline uint32_t _read_word(void* address)
{
  return  *((volatile uint32_t* )( address ));

}





int main() {

int i=0;

  
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_EN,0xf);
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,get_impid());
  wait(3000000); 
  
  
 //_setDivisor(0x340);
 // for(i=0;i<3;i++) {
  while(1) { 
    _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL, 1 << (i++ % 8 ) );  
    wait(1000000);
    if ((i % 8) == 0) {
      setBaudRate(38400);
      writestr("Bonfire \n\r");
    } 
  }   
 
 // }
  //writechar('\x1a'); // Simulation end marker
}
