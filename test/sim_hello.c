#include "wildfire.h"
#include "uart.h"

// sample_clk = (f_clk / (baudrate * 16)) - 1
// (96.000.000 / (115200*16))-1 = 51,08


int main() {

int i;

  setBaudRate(115200);
  for(i=0;i<3;i++) {
    writestr("Bonfire ");
  }
  writechar('\x1a'); // Simulation end marker
}
