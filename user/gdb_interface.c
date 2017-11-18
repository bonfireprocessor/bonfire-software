#include "bonfire.h"

#include <stdio.h>

#include "console.h"

#define UART_TX 0
#define UART_RECV 0
#define UART_STATUS 1
#define UART_CONTROL 1




static volatile uint32_t *uartadr=(uint32_t *)UART1_BASE;


int getDebugChar() {

 while (!(uartadr[UART_STATUS] & 0x01)); // Wait while receive buffer empty
 return uartadr[UART_RECV] & 0x0ff;

};

void putDebugChar(int c) {
  while (!(uartadr[UART_STATUS] & 0x2)); // Wait while transmit buffer full
  uartadr[UART_TX]=(uint32_t)c;
};


static void setDivisor(uint32_t divisor){


   uartadr[UART_CONTROL]= 0x010000L | (uint16_t)divisor; // Set Baudrate divisor and enable port
}

static void setBaudRate(int baudrate) {
// sample_clk = (f_clk / (baudrate * 16)) - 1
// (96.000.000 / (115200*16))-1 = 51,08

   setDivisor(SYSCLK / (baudrate*16) -1);
}



void exceptionHandler(int exception_number,void *execption_address)
{
  // TODO
};

extern void __trap();


void do_increment(volatile int *px)
{
  (*px)++;
}

void main()
{
volatile int i=0;

  setBaudRate(500000);
  printk("Run\n");


  write_csr(mtvec,__trap);
  asm("sbreak");
  while(1) {

    do_increment(&i);
    if ((i % 10000)==0 ) printk("*");
  }
}

