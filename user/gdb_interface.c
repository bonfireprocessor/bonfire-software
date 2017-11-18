#include "bonfire.h"


#define UART_TX 0
#define UART_RECV 0
#define UART_STATUS 1
#define UART_CONTROL 1


volatile uint32_t *uartadr=(uint32_t *)UART_BASE;


int getDebugChar() {

 while (!(uartadr[UART_STATUS] & 0x01)); // Wait while receive buffer empty
 return uartadr[UART_RECV] & 0x0ff;

};

void putDebugChar(int c) {
  while (!(uartadr[UART_STATUS] & 0x2)); // Wait while transmit buffer full
  uartadr[UART_TX]=(uint32_t)c;
};


void exceptionHandler(int exception_number,void *execption_address)
{
  // TODO
};

extern void __trap();

void main()
{
volatile int i=0;

  write_csr(mtvec,__trap);
  while(1) {
    asm("sbreak");
    i++;

  }
}

