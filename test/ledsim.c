#include <stdint.h>
#include "platform.h"
#include "mem_rw.h"

// GPIO Test Program intented for use with the simulator



int main(int argc,char ** argv) {

    uint32_t counter =0;

   _write_word((void*)GPIO_BASE,counter); // Use I/O Reg as Counter variable

    while(1) {
         counter=_read_word((void*)GPIO_BASE);
        _write_word((void*)LED_BASE, (counter++) & 0xff); 
        _write_word((void*)GPIO_BASE,counter);       
   }

};
