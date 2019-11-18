#include "bonfire.h"
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



inline void _write_word(void* address,uint32_t value)
{
  *(( volatile uint32_t* )( address ))=value;
}



int main() {

int i=0;

  _write_word((void*)MONITOR_BASE+4,1);
   setBaudRate(115200);
  _write_word((void*)MONITOR_BASE+4,2);
  //writestr("S\n");
  for(i=0;i<3;i++) {
    writestr("Step ");
  }
  _write_word((void*)MONITOR_BASE+4,3);
  writechar('\x1a'); // Simulation end marker
  _write_word((void*)MONITOR_BASE,3);

}
