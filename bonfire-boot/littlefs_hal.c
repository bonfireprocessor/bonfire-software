#include "bonfire.h"
#include "monitor.h"
#include "console.h"

#if (!defined (NO_FLASH))

#include "littlefs_hal.h"
#include <ctype.h> 

// variables used by the filesystem
lfs_t lfs;





static spiflash_t* spi;


static int lfshal_flash_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
   uint32_t addr =  block * BLOCK_SIZE + FIRST_BLOCK + off;
   return SPIFLASH_read(spi,addr,size,(uint8_t *)buffer);   
}



static int lfshal_flash_prog(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{

   
    uint32_t addr = block * BLOCK_SIZE + FIRST_BLOCK + off;

    printk("lfshal_flash_prog:  block %d, address %lx, size %d\n",block,addr,size);
    return SPIFLASH_write(spi,addr,size,(uint8_t*)buffer);    
}


static int lfshal_flash_erase(const struct lfs_config *c, lfs_block_t block)
{

  printk("lfshal_flash_erase: block %d\n",block);
  return SPIFLASH_erase(spi,block * BLOCK_SIZE + FIRST_BLOCK,BLOCK_SIZE); 
   
}

static int lfshal_flash_sync(const struct lfs_config *c)
{
  return 0;
}

// static int lfshal_flash_lock(const struct lfs_config *c)
// {
//     //rt_mutex_take(&lfs_mutex,RT_WAITING_FOREVER);
//     return 0;
// }


// static int lfshal_flash_unlock(const struct lfs_config *c)
// {
//     //rt_mutex_release(&lfs_mutex);
//     return 0;
// }


const struct lfs_config cfg = {
    // block device operations
    .read  = lfshal_flash_read,
    .prog  = lfshal_flash_prog,
    .erase = lfshal_flash_erase,
    .sync  = lfshal_flash_sync,
    //.lock  = lfshal_flash_lock,
    //.unlock = lfshal_flash_unlock,

    // block device configuration
    .read_size = PAGE_SIZE,
    .prog_size = PAGE_SIZE,
    .block_size = BLOCK_SIZE,
    .block_count = NUM_BLOCKS,
    .cache_size = BLOCK_SIZE,
    .lookahead_size = 256*8,
    .block_cycles = 500,
};


const struct lfs_config *get_lfs_hal()
{
   return &cfg;
}

int do_format(bool unmount)
{
    if (unmount) lfs_unmount(&lfs);
    printk("Formating littleFS\n");
    int err=lfs_format(&lfs, &cfg);
    if (err) {
      printk("LittleFS Format failure %ld\n",err);
      return err;
    }
   
    return  lfs_mount(&lfs, &cfg);  
}

int lfs_init(spiflash_t* _spi)
{
  
      spi = _spi;

      //mount the filesystem
      int err = lfs_mount(&lfs, &cfg);

      // reformat if we can't mount the filesystem
      // this should only happen on the first boot
      if (err) {
           do_format(false);
      }

    return 1;

}

#endif 