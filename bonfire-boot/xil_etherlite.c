/*
 * eLua Platform network interface for Xilinx AXI Ethernet Lite MAC
 * (c) 2017 Thomas Hornschuh
 *
 */


#include "bonfire.h"
#include "pico_stack.h"
#include "xil_etherlite.h"
#include "mem_rw.h"

#include <string.h>
#include <stdbool.h>



#define ETH_BUFSIZE_WORDS (0x07f0/4)

#define MAX_FRAME 1522

// Buffer Offsets
#define O_ETHERTYPE 12 
#define O_PAYLOAD 14 // Begin if Payload
#define O_IP_LENGTH (O_PAYLOAD+2)

// IP Constants
#define ARP_LENGTH 26 // Size of ARP packet 
#define ETYPE_IP 0x800
#define ETYPE_ARP 0x806

#define dbg(...) 

static volatile int in_ethernet_irq = 0;



inline void _write_leds(uint8_t value)
{
   _write_word(( void* )( ARTY_LEDS4TO7+4 ),0); // Set Output Mode
   _write_word(( void* )ARTY_LEDS4TO7,value);
}

inline uint32_t _read_leds()
{
   return _read_word(( void* )ARTY_LEDS4TO7);
}


void platform_eth_init()
{
// {0,0,0x5E,0,0xFA,0xCE}
// static struct uip_eth_addr sTempAddr = {
//     .addr[0] = 0,
//     .addr[1] = 0,
//     .addr[2] = 0x5e,
//     .addr[3] = 0,
//     .addr[4] = 0x0fa,
//     .addr[5] = 0x0ce
//  };

  dbg("Initalizing Ethernet core\n");

  //set_csr(mie,MIP_MEIP); // Enable External Interrupt

  // clear pending packets, enable receive interrupts
  _write_word(ETHL_RX_PING_CTRL,0x8);
  _write_word(ETHL_RX_PONG_CTRL,0x0);
  _write_word(ETHL_TX_PING_CTRL,0);
  //_write_word(ETHL_GIE,0x80000000); // Enable Ethernet Interrupts
}


// Copy dwords between Buffers. Because the Etherlite core don't
// support byte writes, the code ensures to do only 32 Bit transfers
// Attentions: Because it rounds up, it can transfer up to three bytes more
// then specified in size
void eth_copy_buffer(void* dest, const void* src, size_t size)
{
const uint32_t *psrc = src;
uint32_t *pdest = dest;

size_t szwords =(size % 4)?size/4+1:size/4; // round up
int i;

    //printk("copy from %x to %x, %d bytes, %d words\n",src,dest,size,szwords);
    if (szwords>ETH_BUFSIZE_WORDS) {
      dbg("panic: Ethernet buffer copy size overflow: %d\n",szwords);
    }
    for(i=0;i<szwords;i++)
       pdest[i]=psrc[i];
}


void platform_eth_send_packet( const void* src, uint32_t size )
{

   while (_read_word(ETHL_TX_PING_CTRL) & 0x01); // Wait until buffer ready

   eth_copy_buffer(ETHL_TX_PING_BUFF,src,size);
   _write_word(ETHL_TX_PING_LEN,size);
   _write_word(ETHL_TX_PING_CTRL,0x01); // Start send

}


static inline bool isFull( uintptr_t buff )
{
  return _read_word( (void*)( buff+ETHL_OFFSET_CTRL )) & 0x01;
}

// Get Word in Network byte order
static  inline uint16_t get_nbo_word( uintptr_t buff,int offset )
{
  uint8_t *b = (uint8_t*)buff;

  return b[offset+1] | (b[offset] << 8);
}

uint32_t platform_eth_get_packet_nb( void* buf, uint32_t maxlen )
{
// static bool is_PingBuff = true;      // start always with the ping buffer
static uintptr_t currentBuff = (uintptr_t) ETHL_RX_PING_BUFF; // start always with the ping buffer
int length;


  // Check if buffers are out of sync...
  if (!isFull(currentBuff) && isFull(currentBuff ^ PONG_BUFF_OFFSET)) {
    dbg("Etherlite buffers out of sync, correcting...\n");
    currentBuff ^= PONG_BUFF_OFFSET;
  }

  if (isFull(currentBuff)) {
     //dbg("Ethernet Buffer %d used\n",(currentBuff & PONG_BUFF_OFFSET)?1:0);
     if (currentBuff & PONG_BUFF_OFFSET)
      _write_leds(0x01<<3); // light LED7
     else
       _write_leds(0x01<<2); // light LED6

    // Caclucate frame size
    uint16_t ethertype = get_nbo_word(currentBuff,O_ETHERTYPE);
    dbg("Ethertype %x\n",ethertype);
    switch (ethertype) {
      case ETYPE_IP:
        length=get_nbo_word(currentBuff,O_IP_LENGTH) + O_PAYLOAD + 4;
        break;
      case ETYPE_ARP:
        length=O_PAYLOAD+ARP_LENGTH+4;
        break;
      default:
        length=ethertype>MAX_FRAME?MAX_FRAME:ethertype;  
    }   
    if (length>maxlen) length=maxlen;

     memcpy(buf,(void*)currentBuff,length);
     _write_word((void*)currentBuff+ETHL_OFFSET_CTRL,0x8); // clear buffer, enable interrupts
    //  int i;
    //  for(i=0;i<16;i++) dbg("%x ",((uint8_t*)buf)[i]);
    //  dbg("\n");
     currentBuff ^= PONG_BUFF_OFFSET;
     return length;

  } else {
      return 0;
  }
}
// void platform_eth_force_interrupt(void)
// {
// // force_interrupt is called from non-interrupt code
// // so we need to disable interrupts to avoid a real IRQ happening at the same time

// int oldstate=platform_cpu_set_global_interrupts(PLATFORM_CPU_DISABLE);

//   _write_leds(0x02); // light LED5
//   elua_uip_mainloop();
//    _write_leds(0x0);
//   platform_cpu_set_global_interrupts(oldstate);
// }

// u32 platform_eth_get_elapsed_time(void)
// {

//     if( eth_timer_fired )
//     {
//       eth_timer_fired = 0;
//       return 1000/VTMR_FREQ_HZ; // time must be returned in ms !!!
//     }
//     else
//       return 0;

// }



// void ethernet_irq_handler()
// {
//    if (_read_word((void*)BONFIRE_SYSIO) & 0x01) { // Pending IRQ

// #ifdef  BUILD_UIP
//       _write_leds(0x01); // light LED4
//       in_ethernet_irq=1;
//       elua_uip_mainloop();
//       in_ethernet_irq=0;
//       _write_word((void*)BONFIRE_SYSIO,0x01); // clear IRQ
//       _write_leds(0x0);

// #endif
//    } else
//      printk("Uups, ethernet irq handler called without pending IRQ\n");


// }

