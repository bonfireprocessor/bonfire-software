#include <stdint.h>
#include <stdbool.h>
#include "bonfire.h"
#include "monitor.h"
#include "console.h"


#if (!defined (NO_FLASH))
#include "spiflash.h"
#include "spiffs.h" 
#include "spiffs_config.h"




spiffs fs;
spiffs_config fs_config;

static uint8_t work_buffer[SPIFFS_CFG_LOG_PAGE_SZ()*2];
static uint8_t fd_space[256];
static spiflash_t* spi;

static s32_t hal_spiffs_read(u32_t addr, u32_t size, u8_t *dst)
{
    return SPIFLASH_read(spi,addr,size,dst);
}

static s32_t hal_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
    return SPIFLASH_write(spi,addr,size,src);
}
static s32_t hal_spiffs_erase(u32_t addr, u32_t size)
{
    return SPIFLASH_erase(spi,addr,size);
}

int32_t spiffs_init(spiflash_t *_spi,uint32_t cache_size)
{
uint8_t * cache = malloc(cache_size);
int32_t result;    

    spi = _spi;

    fs_config.hal_read_f=hal_spiffs_read;
    fs_config.hal_write_f=hal_spiffs_write;
    fs_config.hal_erase_f=hal_spiffs_erase;

    result=SPIFFS_mount(&fs,&fs_config,work_buffer,fd_space,sizeof(fd_space),cache,cache_size,NULL);
    if (result==SPIFFS_ERR_NOT_A_FS) {
        printk("No filesystem found, formating...");
        result=SPIFFS_format(&fs);
        printk("SPIFFS format result: %ld\n",result);
        if (result==SPIFFS_OK) {
           result=SPIFFS_mount(&fs,&fs_config,work_buffer,fd_space,sizeof(fd_space),cache,cache_size,NULL);
        }
    }
    printk("SPIFFS Mount result: %ld\n",result);
    return result;
}


bool handle_error(int code)
{
  if(code<0) {
      printk("SPIFFS error %d\n",SPIFFS_errno(&fs));
      return false;
  } else {
      return true;
  }
}

int32_t spiffs_save(char *filename,void *memptr,uint32_t size)
{
spiffs_file fd = SPIFFS_open(&fs,filename,SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR,0);
int r;

    if (!handle_error(fd)) return fd;
    r = SPIFFS_write(&fs,fd,memptr,size);
    if (!handle_error(r)) return r;
    r = SPIFFS_close(&fs,fd);
    handle_error(r);
    return r;
}


#endif