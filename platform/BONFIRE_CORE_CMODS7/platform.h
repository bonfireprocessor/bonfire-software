#ifndef PLARFORM_BONFIRE_CORE_CMODS7_PLATFORM_H
#define PLARFORM_BONFIRE_CORE_CMODS7_PLATFORM_H 

#include "../BONFIRE_CORE/platform.h"

#undef SYSCLK
#define SYSCLK 96000000  // 96 MHz

#undef CLK_PERIOD
#define CLK_PERIOD (1e+9 / SYSCLK)  // in ns...

// #undef SRAM_SIZE
// #define SRAM_SIZE (4096*4)
// #undef SRAM_TOP
// #define SRAM_TOP  (SRAM_BASE+SRAM_SIZE-1)


#endif
