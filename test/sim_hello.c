#include "wildfire.h"
#include "uart.h"




int main() {

int i;

  setBaudRate(38400);
 // for(i=0;i<3;i++) {
    writestr("Bonfire ");
 // }
  writechar('\x1a'); // Simulation end marker
}
