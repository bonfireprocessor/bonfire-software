#ifndef __LITTLEFS_HAL_H
#define __LITTLEFS_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "spiflash.h"
#include "lfs.h"

extern lfs_t lfs;
int lfs_init(spiflash_t* _spi);

int do_format(bool unmount);



#endif