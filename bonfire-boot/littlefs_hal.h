#ifndef __LITTLEFS_HAL_H
#define __LITTLEFS_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "spiflash.h"
#include "lfs.h"
#include <string.h>


#define FIRST_BLOCK FLASH_FSBASE // (1024*6144) // Start at 6MB - for testing

#if ( defined(NO_SUB_SECTOR_ERASE) && NO_SUB_SECTOR_ERASE==1)
#define BLOCK_SIZE 65536
#else
#define BLOCK_SIZE 4096
#endif

#define FS_SIZE FLASH_FSSIZE // (2048*1024) // 2MB -- for tesing
#define NUM_BLOCKS (FS_SIZE/BLOCK_SIZE)
#define PAGE_SIZE 16

extern lfs_t lfs;
int lfs_init(spiflash_t* _spi);

int do_format(bool unmount);

static inline void fd_init(lfs_file_t *fd)
{
   memset((void*)fd,0,sizeof(lfs_file_t));
}




#endif