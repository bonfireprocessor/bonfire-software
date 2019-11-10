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

  setBaudRate(38400);
  for(i=0;i<3;i++) { 
      writestr("Bonfire \n\r");
  } 
  
  writechar('\x1a'); // Simulation end marker
}
