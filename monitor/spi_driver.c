#include "bonfire.h"
#include "console.h"
#include "spiflash.h"
#include "monitor.h"

#if (!defined (NO_FLASH))

/*
-- registers:
-- base+0   -- chip select control; bit 0 is slave_cs
-- base+4   -- status register; bit 0 indicates "transmitter busy"
-- base+8   -- transmitter: write a byte here, starts SPI bus transaction
-- base+0x0C   -- receiver: last byte received (updated on each transation)
-- base+0x10   -- clock divider: SPI CLK is clk_i/2*(1+n) ie for 96MHz clock, divisor 0 is 48MHz, 2 is 24MHz, 3 is 12MHz etc
*/

#define SPI_CHIPSELECT  0
#define SPI_STATUS      1
#define SPI_TX          2
#define SPI_RX          3
#define SPI_DIVISOR     4



#define FLASH_MAN      0xc2
#define FLASH_DEV1     0x20
#define FLASH_DEV2     0x17





typedef uint8_t t_flashid[3];

static volatile uint32_t *spiflash = (void*)SPIFLASH_BASE;

static inline void spiflash_init()
{
   spiflash[SPI_DIVISOR]=1;

}

static inline void spiflash_select()
{
  spiflash[SPI_CHIPSELECT]=0x0fe;
}

static inline void spiflash_deslect()
{
    spiflash[SPI_CHIPSELECT]=0x0ff;
}


static inline void spi_tx(uint8_t txbyte)
{
   spiflash[SPI_TX]=txbyte;
}

static inline uint8_t spi_rx()
{
  spiflash[SPI_TX]=0; // send dummy byte
  return (uint8_t) (spiflash[SPI_RX] & 0x0ff);
}




void spiflash_getid(t_flashid *pflashid)
{
  spiflash_select();
  spi_tx(0x9f);      //identify/RDID command

  spi_tx(0);
  spi_tx(0);
  spi_tx(0);
  (*pflashid)[0]=spi_rx();
  (*pflashid)[1]=spi_rx();
  (*pflashid)[2]=spi_rx();

  spiflash_deslect();

}


void delay_1ms()
{
  delay_loop(1000000 / LOOP_TIME);
}



int impl_spiflash_spi_txrx(spiflash_t *spi, const uint8_t *tx_data,
      uint32_t tx_len, uint8_t *rx_data, uint32_t rx_len) {
  int res = SPIFLASH_OK;
  int i;



  if (tx_len > 0) {
    // first transmit tx_len bytes from tx_data if needed
    for(i=0;i<tx_len;i++) spi_tx(tx_data[i]);
  }

  if (rx_len > 0) {
    // then receive rx_len bytes into rx_data if needed
    for(i=0;i<rx_len;i++) rx_data[i]=spi_rx();
  }

  return res;
}


void impl_spiflash_spi_cs(spiflash_t *spi, uint8_t cs) {
  if (cs) {
    // assert cs pin
    spiflash_select();
  } else {
    // de assert cs pin
   spiflash_deslect();
  }
}


void impl_spiflash_wait(spiflash_t *spi, uint32_t ms) {
  while(ms) {
    delay_1ms();
    ms--;
  }
}

static const spiflash_hal_t my_spiflash_hal = {
  ._spiflash_spi_txrx = impl_spiflash_spi_txrx,
  ._spiflash_spi_cs = impl_spiflash_spi_cs,
  ._spiflash_wait = impl_spiflash_wait
};


#define SPIFLASH_CMD_TBL_64K \
  (spiflash_cmd_tbl_t) { \
    .write_disable = 0x04, \
    .write_enable = 0x06, \
    .page_program = 0x02, \
    .read_data = 0x03, \
    .read_data_fast = 0x0b, \
    .write_sr = 0x01, \
    .read_sr = 0x05, \
    .block_erase_4 = 0x00, \
    .block_erase_8 = 0x00, \
    .block_erase_16 = 0x00, \
    .block_erase_32 = 0x00, \
    .block_erase_64 = 0xd8, \
    .chip_erase = 0xc7, \
    .device_id = 0x90, \
    .jedec_id = 0x9f, \
    .sr_busy_bit = 0x01, \
  }

#if (defined(NO_SUB_SECTOR_ERASE) && NO_SUB_SECTOR_ERASE==1 )
#pragma message "SPI Flash using SPIFLASH_CMD_TBL_64K"
const spiflash_cmd_tbl_t my_spiflash_cmds = SPIFLASH_CMD_TBL_64K;
#ifndef FLASH_ERASEBLOCK
#define FLASH_ERASEBLOCK 65536
#endif 
#else
const spiflash_cmd_tbl_t my_spiflash_cmds = SPIFLASH_CMD_TBL_STANDARD;
#ifndef FLASH_ERASEBLOCK
#define FLASH_ERASEBLOCK 4096
#endif 
#endif

const spiflash_config_t my_spiflash_config = {
  .sz = 1024*1024*8, // 8MB
  .page_sz = 256, // normally 256 byte pages
  .addr_sz = 3, // normally 3 byte addressing
  .addr_dummy_sz = 0, // using single line data, not quad or something
  .addr_endian = SPIFLASH_ENDIANNESS_BIG, // normally big endianess on addressing
  .sr_write_ms = 10,
  .page_program_ms = 2,
  .block_erase_4_ms = 100,
  .block_erase_8_ms = 0, // not supported
  .block_erase_16_ms = 0, // not supported
  
  .block_erase_32_ms = 0, // 175, 
  .block_erase_64_ms = 300,
  .chip_erase_ms = 30000
};

static spiflash_t spif;



/*
bool spiflash_test()
{
t_flashid id;

//int i;

    //printk("wait 5 Seconds...");
    //for(i=0;i<5000;i++) delay_1ms();
    //printk("ok\n");

    spiflash_init();
    spiflash_getid(&id);
    printk("Flash id %x %x %x\n",id[0],id[1],id[2]);

    return (id[0]==FLASH_MAN && id[1]==FLASH_DEV1 && id[2]==FLASH_DEV2);

}
*/




spiflash_t* flash_init()
{
uint8_t jedec_id[4];

   spiflash_init();
   //spiflash_test();
   SPIFLASH_init(&spif,
                &my_spiflash_config,
                &my_spiflash_cmds,
                &my_spiflash_hal,
                0,
                SPIFLASH_SYNCHRONOUS,
                NULL);

    SPIFLASH_read_jedec_id(&spif,(uint32_t*)&jedec_id);
    printk("Flash Man. ID: %x Dev. ID: %x\n",jedec_id[0],jedec_id[1]<<8 | jedec_id[2]);
    return &spif;
}

int flash_print_spiresult(int code)
{
  if (code==SPIFLASH_OK)
    printk("...OK\n");
  else
    printk("SPIFLASH err %x\n",code);

   return code;
}


bool check_erase(void *buffer,int len_words)
{
uint32_t *p=(uint32_t*)buffer;

    //printk("%x: %x\n",buffer,*p);
    for (int i=0;i<len_words;i++) {
      if (*p++ != 0xffffffff) return false;
    }
    return true; 
}

static int flash_write_verify(spiflash_t *spi,uint32_t addr,const uint8_t *buf, uint32_t len, uint8_t* compare_buffer)
{
int res;

    // Check for sucessfull erase
    SPIFLASH_read(spi,addr,len,compare_buffer);
    if (!check_erase(compare_buffer,len/4)) {
      printk("flash at %x not erased\n",addr);
      return SPIFLASH_ERR_INTERNAL;
      
    }
    printk("Writing mem %x to flash %x\n",buf,addr);
    res=SPIFLASH_write(spi,addr,len,buf);
    if (res!=SPIFLASH_OK) return res;
    res=SPIFLASH_read(spi,addr,len,compare_buffer);
    if (res!=SPIFLASH_OK) return res;
    printk("Comparing...\n");
    if (memcmp(buf,compare_buffer,len)!=0) {
      return SPIFLASH_ERR_INTERNAL; // not better error code for the moment... 
    }	 
    return SPIFLASH_OK;
}

#define FLASH_WRITE_SIZE 256

int flash_Overwrite(spiflash_t *spi, uint32_t addr,const uint8_t *buf, uint32_t len,t_flash_header* header)
{
int res;
uint8_t *compare_buffer=(uint8_t*) (DRAM_TOP & 0xffff0000);


  printk("Compare Buffer at %x\n",compare_buffer);

  if (header) len += sizeof(t_flash_header); // Wenn a header is passed at to len

   // Round up erase size by erase block size 
  int nBlocks = len / FLASH_ERASEBLOCK;
   if (len % FLASH_ERASEBLOCK) nBlocks++;
   
  printk("Erasing %d %dKB Blocks at %x...\n",nBlocks,FLASH_ERASEBLOCK/1024,addr);
  res=SPIFLASH_erase(spi,addr,nBlocks*FLASH_ERASEBLOCK);
  flash_print_spiresult(res);
  if (res!=SPIFLASH_OK) return res;


  // Recaclculate write block size
  nBlocks = len / FLASH_WRITE_SIZE;
  if (len % FLASH_WRITE_SIZE) nBlocks++;

  if (header) {
    res=flash_write_verify(spi,addr,(uint8_t*)header, sizeof(t_flash_header),compare_buffer);
    flash_print_spiresult(res);
    if (res!=SPIFLASH_OK) return res;
    len -= sizeof(t_flash_header);
    addr+= sizeof(t_flash_header);
  }

  while(len>0 && res==SPIFLASH_OK) {   
    int wlen = (len>FLASH_WRITE_SIZE)?FLASH_WRITE_SIZE:len;
    res=flash_write_verify(spi,addr,buf,wlen, compare_buffer);
    if (res!=SPIFLASH_OK) {
      printk("SPI Write Error at addr %lx\n",addr);
      break;
    }
    len-=wlen;   
    addr+=wlen; buf+=wlen;
  }

  return flash_print_spiresult(res);

}


#endif 

