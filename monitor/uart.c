/* Test UART */
// sample_clk = (f_clk / (baudrate * 16)) - 1
// (32.000.000 / (115200*16))-1 = 16,36 ...

//    |--------------------|--------------------------------------------|
//--! | Address            | Description                                |
//--! |--------------------|--------------------------------------------|
//--! | 0x00               | Transmit register(W)/  Receive register(R) |
//--! | 0x04               | Status(R)) and control(W) register         |
//--! |--------------------|--------------------------------------------|

//--! The status register contains the following bits:
//--! - Bit 0: UART RX Ready bit. Reads as 1 when there's received data in FIFO, 0 otherwise.
//--! - Bit 1: UART TX Ready bit. Reads as 1 when there's space in TX FIFO for transmission, 0 otherwise.

// Control register:
// Bit [15:0] - UARTPRES UART prescaler (16 bits)   (f_clk / (baudrate * 16)) - 1
// Bit 16 - UARTEN UARTEN bit controls whether UART is enabled or not


#include <stdint.h>
#include <stdbool.h>


#include "platform.h"



#define UART_TX 0
#define UART_RECV 0
#define UART_STATUS 1
#define UART_CONTROL 2


#define ENABLE_SEND_DELAY 1


volatile uint32_t *uartadr=(uint32_t *)UART_BASE;

volatile uint32_t *gpioadr=(uint32_t *)GPIO_BASE;


void wait(long nWait)
{
static volatile int c;

  c=0;
  while (c++ < nWait);
}



void writechar(char c)
{

#ifdef  ENABLE_SEND_DELAY
   wait(1000);
#endif
  while (!(uartadr[UART_STATUS] & 0x2)); // Wait while transmit buffer full
  uartadr[UART_TX]=(uint32_t)c;

}

char readchar()
{
  while (!(uartadr[UART_STATUS] & 0x01)); // Wait while receive buffer empty
  return (char)uartadr[UART_RECV];
}


int wait_receive(long timeout)
{
uint8_t status;
bool forever = timeout < 0;

  do {
    status=uartadr[UART_STATUS];
  //  *gpioadr = status & 0x0f; // show status on LEDs
    if (status & 0x01) { // receive buffer not empty?
   //    *gpioadr=0; // clear LEDs
      return uartadr[UART_RECV];
    } else
      timeout--;

  }while(forever ||  timeout>=0 );
  *gpioadr=0; // clear LEDs
  return -1;

}



void writestr(char *p)
{
  while (*p) {
    writechar(*p);
    p++;
  }
}


// Like Writestr but expands \n to \n\r
void write_console(char *p)
{
   while (*p) {
    if (*p=='\n') writechar('\r');
    writechar(*p);
    p++;
  }

}

void writeHex(uint32_t v)
{
int i;
uint8_t nibble;
char c;


   for(i=7;i>=0;i--) {
     nibble = (v >> (i*4)) & 0x0f;
     if (nibble<=9)
       c=(char)(nibble + '0');
     else
       c=(char)(nibble-10+'A');

     writechar(c);
   }
}


static uint16_t l_divisor=0;

void _setDivisor(uint32_t divisor){

   l_divisor = divisor;
   uartadr[UART_CONTROL]= 0x030000L | (uint16_t)divisor; // Set Baudrate divisor and enable port and set extended mode
}

void setDivisor(uint32_t divisor)
{
    _setDivisor(divisor);
    wait(1000000);
}

uint32_t getDivisor()
{
  return  uartadr[UART_CONTROL] & 0x0ffff ;
}

void setBaudRate(int baudrate) {


   setDivisor(SYSCLK / baudrate -1);
}

uint8_t getUartRevision()
{
   return 0x0ff; // not supported with ZPUINO UART yet

}
