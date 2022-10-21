#ifndef __LITTLEFS_HAL_H
#define __LITTLEFS_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "spiflash.h"
#include "lfs.h"
#include <string.h>

extern lfs_t lfs;
int lfs_init(spiflash_t* _spi);

int do_format(bool unmount);

static inline void fd_init(lfs_file_t *fd)
{
   memset((void*)fd,0,sizeof(lfs_file_t));
}




#endif