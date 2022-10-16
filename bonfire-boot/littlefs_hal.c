#include "bonfire.h"
#include "monitor.h"
#include "console.h"

#if (!defined (NO_FLASH))

#include "lfs.h"
#include <stdbool.h>
#include <ctype.h> 
#include "spiflash.h"





// variables used by the filesystem
lfs_t lfs;


#define FIRST_BLOCK  (1024*6144) // Start at 6MB - for testing
#define BLOCK_SIZE 65536
#define FS_SIZE (1024*1024) // 1MB -- for tesing
#define NUM_BLOCKS (FS_SIZE/BLOCK_SIZE)
#define PAGE_SIZE 16


static spiflash_t* spi;


static int lfshal_flash_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
   uint32_t addr =  block * BLOCK_SIZE + FIRST_BLOCK + off;
   return SPIFLASH_read(spi,addr,size,(uint8_t *)buffer);   
}


// static HAL_StatusTypeDef _flash_program_verify(rt_uint32_t dest,void* src)
// {
//     //rt_kprintf("Program quadword at %lx\n",dest);
//     HAL_StatusTypeDef res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD,dest,(uint32_t)src);
//     //rt_kprintf("HAL_Status: %lx\n",res);
//     if (res!=HAL_OK) return res;

//     rt_uint32_t * d = (rt_uint32_t*)dest;
//     rt_int32_t * s = (rt_uint32_t*)src;

//     HAL_ICACHE_Invalidate();
//     for(int i=0;i<4;i++) {
//         if (*d != *s) {
//             rt_kprintf("Flash program error at %lx (%lx != %lx)\n",d,*d,*s);
//             return HAL_ERROR;
//         }
//     }
//     return HAL_OK;
// }


static int lfshal_flash_prog(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{

   
    uint32_t addr = block * BLOCK_SIZE + FIRST_BLOCK + off;

    printk("lfshal_flash_prog to block %d, address %lx, size %d\n ",block,addr,size);
    return SPIFLASH_write(spi,addr,size,(uint8_t*)buffer);    
}


static int lfshal_flash_erase(const struct lfs_config *c, lfs_block_t block)
{

  printk("lfshal_flash_erase with block %d\n",block);
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



int lfs_init(spiflash_t* _spi)
{
//lfs_file_t file;
   
      //mount the filesystem
      spi = _spi;

      int err = lfs_mount(&lfs, &cfg);

      // reformat if we can't mount the filesystem
      // this should only happen on the first boot
      if (err) {
          printk("Formating littleFS\n");
          err=lfs_format(&lfs, &cfg);
          if (err) {
            printk("LittleFS Format failure %ld\n",err);
            return -1;
          }
          lfs_mount(&lfs, &cfg);
      }

   
    //   uint32_t boot_count = 0;
    //   lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    //   lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    //   // update boot count
    //   boot_count += 1;
    //   lfs_file_rewind(&lfs, &file);
    //   lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    //   // remember the storage is not updated until the file is closed successfully
    //   lfs_file_close(&lfs, &file);
    //   printk("boot_count: %d\n", boot_count);
    // //}
    // // release any resources we were using
    // lfs_unmount(&lfs);

    return 1;

}

#endif 