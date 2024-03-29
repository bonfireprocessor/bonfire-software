#include "bonfire.h"

#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "monitor.h"
#include "mempattern.h"
#include "console.h"
#include "xmodem.h"
#include "syscall.h"
#include "mem_rw.h"
#include "bonfire_gpio.h"

#include "spi_driver.h"


#if GDBSTUB==1
#include "riscv-gdb-stub.h"
static bool gdb_status = false;
#endif


#define LOAD_SIZE  (DRAM_SIZE-(long)LOAD_BASE)


 // Important: Stack must be aligned modulo 8, otherwiese the varargs of doubles did not work
// Searched for this nearly a day, wondering why printf of doubles did not work...
#define USER_STACK (DRAM_TOP & 0x0fffffff8)


#ifdef PLATFORM_BAUDRATE
  #define BAUDRATE PLATFORM_BAUDRATE
#else   
    #define BAUDRATE 500000L
#endif

//#define BAUDRATE 115200L

// XModem and spi Flash Variables

static long recv_bytes=0;

extern uint32_t  brk_address;


static void handle_syscall(trapframe_t* tf)
{
#if (!defined (NO_SYSCALL))  
  tf->gpr[10] = do_syscall(tf->gpr[10], tf->gpr[11], tf->gpr[12], tf->gpr[13],
                           tf->gpr[14], tf->gpr[15], tf->gpr[17]);
#else
  tf->gpr[10] = -38; // ENOSYS
#endif                           
  tf->epc += 4;
}


trapframe_t* trap_handler(trapframe_t *ptf)
{
char c;


    //*gpioadr = (ptf->cause & 0x0f); // Show trap cause on LEDs

    if (ptf->cause==11 || ptf->cause==8) // ecall
        handle_syscall(ptf);
    else {

  #if (GDBSTUB==1)
        if (gdb_status) {
             return handle_exception(ptf);
        }
  #endif
        printk("\nTrap cause: %lx\n",ptf->cause);
        dump_tf(ptf);
        c=readchar();
        if (c=='r' || c=='R')
          ptf->epc=SRAM_BASE; // will cause reset
        else
          if (((long)ptf->cause)>0) ptf->epc+=4;
    }
    return ptf;
}


void xm_send(u8 c)
{
  writechar((char)c);
}

#if (!defined (NO_DRAMTEST))
void test_dram(uint32_t sz)
{
   printk("\nTesting %d Kilobytes of DRAM...\n",sz/1024);
   writepattern((void*)DRAM_BASE,sz/4);
   printk("Verifying...\n");
   printk("Found %d errors\n",verifypattern((void*)DRAM_BASE,sz/4));
}
#endif 

void flush_dache()
{
#ifdef DCACHE_SIZE
uint32_t *pmem = (void*)(DRAM_TOP-DCACHE_SIZE+1);
static volatile uint32_t sum=0; // To avoid optimizing away code below

  printk("Cache %d bytes Flush read from %lx\n",DCACHE_SIZE,pmem);
  while ((uint32_t)pmem < DRAM_TOP) {
    sum+= *pmem++;
  }
  asm("fence.i"); // Flush also instruction cache

#endif

}


void changeBaudRate()
{
char strbuff[8];
long newBaud;

   printk("\nEnter new baudrate: ");
   read_num_str(strbuff,sizeof(strbuff));
   if (strbuff[0]) {
     newBaud=atol(strbuff);
     if (newBaud>=2400L && newBaud<=2000000L) {
        printk("\nChangine baudratew now....\n");
        setBaudRate(newBaud);
     } else {
       printk("\nInvalid, enter 2400-2000000\n",newBaud);
     }
   }
}


void printInfo()
{


  printk("\nBonfire Boot Monitor 0.5a (20220801) (GCC %s)\n",__VERSION__);
  printk("MIMPID: %lx\nMISA: %lx\nUART Divisor: %d\nUptime %d sec\n",
         read_csr(mimpid),read_csr(misa),
         getDivisor(),sys_time(NULL));

  #ifdef BONFIRE_SYSIO
    printk("SoC Build ID: %lx\n",_read_word((void*)BONFIRE_SYSIO+4));
  #endif          

  printk("DRAM Size %ld bytes\n",DRAM_SIZE);

#if (!defined (NO_DCACHE_TEST))
  print_cache_size();
#endif
  printk("Framing errors: %ld\n",getFramingErrors());
}

void error(int n)
{
  printk("Error %d\n",n);
}

#if (!defined (NO_FLASH))

void writeBootImage(spiflash_t *spi)
{
uint32_t nFlashBytes;

   if (!flash_header.nPages)
     printk("First load Image !");
   else {
    
     nFlashBytes = flash_header.nPages << 12;
     if ((nFlashBytes+sizeof(t_flash_header)) >MAX_FLASH_IMAGESIZE) {
       printk("Image size %d > %d, abort\n",nFlashBytes+4096,MAX_FLASH_IMAGESIZE);
       return;
     }
     printk("Saving Image %lx to Flash Address %x (%d bytes) \n",(uint8_t*)flash_header.loadAddress,
            FLASH_IMAGEBASE,nFlashBytes+sizeof(t_flash_header));
     // TH 220420: Write Header + Image in one step
     flash_Overwrite(spi,FLASH_IMAGEBASE,(uint8_t*)flash_header.loadAddress,nFlashBytes,(t_flash_header*)HEADER_BASE);

   }

}


int readBootImage(spiflash_t *spi)
{
int err;

  printk("Reading Header\n");
  err=SPIFLASH_read(spi,FLASH_IMAGEBASE,sizeof(t_flash_header),HEADER_BASE);
  if (flash_print_spiresult(err)!=SPIFLASH_OK) return err;
  // Check Header
  if (flash_header.magic == C_MAGIC) {
    uint32_t nFlashBytes =  flash_header.nPages << 12;
    printk("Boot Image found, length %d Bytes, load address %lx, Break Address: %x\n",nFlashBytes,
            flash_header.loadAddress, flash_header.brkAddress);
    err=SPIFLASH_read(spi,FLASH_IMAGEBASE+sizeof(t_flash_header),nFlashBytes,(uint8_t*)flash_header.loadAddress);
    if (flash_print_spiresult(err)!=SPIFLASH_OK) return err;
    brk_address= flash_header.brkAddress;
    return SPIFLASH_OK;

  } else {

    printk("Invalid Boot Image header\n");
    return -1;
  }
}

#endif 

#if (defined(GPIO_TEST) )

void gpio_test()
{
int i=0;
int t_old = -1;


   while(wait_receive(0)== -1) { 
      int t = sys_time(NULL);
      if (t!=t_old) {
        _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL, 1 << (i++ % 8 ) );  
        if ((i % 8) == 0) {
          printk("Uptime: %d sec\n",sys_time(NULL));
        }
        t_old = t; 
      }      
  }   

} 
#endif


static void bootFromImage(spiflash_t *spi)
{
   if (readBootImage(spi)==SPIFLASH_OK) {
      flush_dache();
      clear_csr(mstatus,MSTATUS_MIE);

      start_user((uint32_t)flash_header.loadAddress,USER_STACK ); // will not return...
  }
}

int main()
{

char cmd[64];
char *p;
char xcmd;
uint32_t *dumpaddress=LOAD_BASE;



void (*pfunc)();

uint32_t args[3];
int nArgs;


spiflash_t *spi;

uint32_t nFlashbytes;
uint32_t flashAddress;
int err;


   flash_header.magic=0; 
   setBaudRate(BAUDRATE);
/* #ifndef SIM
     wait(1000000);
#endif */

#if (defined(GPIO_TEST) )

  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_EN,0xf);
  _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,get_impid());
#endif


   printInfo();
#if (!defined (NO_FLASH))   
   spi=flash_init();
#endif    

#ifdef ARTY_SWITCHES_BUTTONS

  // Boot automatically into the Flash Image when SW0 on an Arty board is set
  if (_read_word((void*)ARTY_SWITCHES_BUTTONS) & 0x1)   bootFromImage(spi);

#endif


   #ifdef SIM
   writechar(0x1a); // Stop simulation here 
   #endif
   while(1) {
     restart:
     write_console("\n>");
     if (readBuffer(cmd,sizeof(cmd))) {
        p=cmd;
        skipWhiteSpace(&p);
        if (*p!='\0' && isalpha((int)*p)) {
          xcmd=toupper(*p++);
          skipWhiteSpace(&p);
        }  else {
          error(1);
          continue;
        }
        nArgs=0;
        while(nArgs<3 && *p!='\0' ) {
          if (parseNext(p,&p,&args[nArgs])) {
            nArgs++;
            skipWhiteSpace(&p);
          } else {
            error(2);
            goto restart;
          }
        };
     } else continue;

     switch(xcmd) {
       case 'D': // Dump command

         if (nArgs>=1)
             dumpaddress=(uint32_t*) (args[0] & 0x0fffffffc); // mask lower two bits to avoid misalignment
         hex_dump((void*)dumpaddress,64);
         dumpaddress+=64;
         break;
#if (!defined(NO_XMODEM))         
       case 'X': // XModem receive command
         switch(nArgs) {
            case 0:
              args[0]=(uint32_t)LOAD_BASE;
              args[1]=LOAD_SIZE;
              break;
            case 1:
              if (args[0]>=DRAM_TOP) {
                 error(3);
                 continue;
              }
              args[1]=DRAM_SIZE-args[0];
              break;
            default:
              error(4);
              continue;
         };

         write_console("Wait for receive...\n");
         recv_bytes=xmodem_receive((char*)args[0],args[1]);
         if (recv_bytes>0) {
             flash_header.magic = C_MAGIC;
             flash_header.nPages = recv_bytes >> 12; // Number of 4096 Byte pages
             if (recv_bytes % 4096) flash_header.nPages+=1; // Round up..
             flash_header.brkAddress = (args[0] + recv_bytes + 4096) & 0x0fffffffc;
             flash_header.loadAddress = args[0];
             flush_dache();
         } else {
           flash_header.magic = 0;
         }
         
         break;

       case 'E':
         if (recv_bytes>=0)
           printk("\n%ld Bytes received\nload address: %lx\n%d Pages\nBreak Address: %x\n",recv_bytes,
                        flash_header.loadAddress, 
                        flash_header.nPages,flash_header.brkAddress);
         else {
           printk("\nXmodem Error %ld occured\n",recv_bytes);
           xmmodem_errrorDump();
         }
         break;
#endif  
#if (!defined (NO_DRAMTEST))        
       case 'T':
         test_dram(nArgs>=1?args[0]:DRAM_TOP);
         break;
#endif          
      case 'B':
        changeBaudRate();
        break;
      case 'I':
        printInfo();
        break;
       case 'G':
         if (nArgs>=1)
           pfunc=(void*)args[0];
         else
           pfunc=LOAD_BASE;
         clear_csr(mstatus,MSTATUS_MIE);
         start_user((uint32_t)pfunc,USER_STACK );
         break;

#if (!defined (NO_FLASH))
       case 'F': // flash read
         // Usage ftarget_adr,flash_page(4K),len (pages)
         switch(nArgs) {
           case 0: // No Arugments default...
             args[0]=(uint32_t)LOAD_BASE;
             // fall through
           case 1:  // Only Load Base specified
             args[1]=FLASH_IMAGEBASE >> 12; 
             // fall through
           case 2:
             args[2]=0x80; // Load 512KB

         }

         flashAddress=args[1] << 12;
         nFlashbytes=args[2] << 12;
         if (flashAddress>=FLASHSIZE || (flashAddress+nFlashbytes) >=FLASHSIZE  || nFlashbytes==0) {
            printk("Invalid args");
            continue;
          }

         printk("Flash read to %x from Page %p (%d Bytes)...\n",args[0],args[1],nFlashbytes);
         err=SPIFLASH_read(spi,flashAddress,nFlashbytes,(uint8_t*)args[0]);
         if (err!=0)
            error(err);
         else
           printk("OK");
         flush_dache();
         break;

       case 'R': // Load Boot Image from Flash and run
         bootFromImage(spi);
         break;

       case 'W': // flash write
         writeBootImage(spi);
         break;
#endif 

#if (!defined (NO_DCACHE_TEST))
       case 'C':
         test_dcache(nArgs?args[0]:DCACHE_SIZE);
         break;
#endif
#if (defined(GPIO_TEST) )
       case 'P':
          gpio_test();
          break; 
#endif 
#if (GDBSTUB==1)
       case 'Z':
         if (!nArgs) args[0]=BAUDRATE;
         printk("Connect debugger with %ld baud\n",args[0]);
         gdb_setup_interface(args[0]);
         gdb_status=true;
         gdb_breakpoint();
         break; 
#endif 
       default:
         printk("\a?\n"); // beep...
      }

   }
}
