#ifndef __SPIFFS_HAL_H
#define __SPIFFS_HAL_H

#include <stdint.h>
#include "spiflash.h"

int32_t spiffs_init(spiflash_t *_spi,uint32_t cache_size);
int32_t spiffs_save(char *filename,void *memptr,uint32_t size);

#endif