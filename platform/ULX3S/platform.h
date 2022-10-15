#ifndef ULX3S_PLATFORM_H
#define ULX3S_PLATFORM_H

#pragma message "ULX3S platform"

// New Defintions for new bonfire-soc-io core
#define IO_BASE 0x04000000
#define SOC_IO_OFFSET 0x10000 // Offset from one I/O Device to the next (64K range)

#define UART0_BASE IO_BASE
#define SPIFLASH_BASE (IO_BASE+SOC_IO_OFFSET)
#define GPIO_BASE (IO_BASE+3*SOC_IO_OFFSET)
#define UART1_BASE (IO_BASE+2*SOC_IO_OFFSET)

#define UART_BASE UART0_BASE // Backwards compatiblity

#define MTIME_BASE 0x0FFFF0000

#define DRAM_BASE 0x0
#define DRAM_SIZE (32*1024*1024)
#define DRAM_TOP  (DRAM_BASE+DRAM_SIZE-1)
#define SRAM_BASE 0xC0000000
#define SRAM_SIZE 8192
#define SRAM_TOP  (SRAM_BASE+SRAM_SIZE-1)

//#define SYSCLK 96000000
#define SYSCLK 62500000  // 62.5 MHz 

#define CLK_PERIOD (1e+9 / SYSCLK)  // in ns...

#define DCACHE_SIZE 16384 //  DCache Size in Bytes


// Parameters for SPI Flash

#define FLASHSIZE (8192*1024)
#define MAX_FLASH_IMAGESIZE (2024*1024) // Max 2MB of flash used for boot image
#define FLASH_IMAGEBASE (1024*6144)  // Boot Image starts at 6MB in Flash

#define NO_SUB_SECTOR_ERASE 1 // Flash chip does not support sub-sector erase (4K bytes), e.g. Spansion on Arty REV E.
 
#define PLATFORM_BAUDRATE 115200
//#define PLATFORM_BAUDRATE 500000

#define NO_SYSCALL
//#define NO_FLASH
//#define NO_XMODEM
//#define NO_DRAMTEST
#define NO_DCACHE_TEST
#define GPIO_TEST

#endif
