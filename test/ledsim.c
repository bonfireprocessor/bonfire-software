#include <stdint.h>
#include "platform.h"
#include "mem_rw.h"

// GPIO Test Program intented for use with the simulator


int main(int argc,char ** argv) {

   volatile  counter =0;

    while(1) {
	
         _write_word((void*)GPIO_BASE,( counter++ >> 2 )  & 0x0f);       
   }

};
