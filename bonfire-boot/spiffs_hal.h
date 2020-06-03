#ifndef __SPIFFS_HAL_H
#define __SPIFFS_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "spiflash.h"
#include "spiffs.h"

extern spiffs fs;
extern spiffs_config fs_config;

int32_t spiffs_init(spiflash_t *_spi,uint32_t cache_size,bool autoformat);
int32_t spiffs_save(char *filename,void *memptr,uint32_t size);


#endif