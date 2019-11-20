#ifndef BASIC_PLATFORM_H
#define BASIC_PLATFORM_H


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
#define DRAM_SIZE 0
#define DRAM_TOP  (DRAM_BASE+DRAM_SIZE-1)
#define SRAM_BASE 0x0C000000
#define SRAM_SIZE 2048
#define SRAM_TOP  (SRAM_BASE+SRAM_SIZE-1)

//#define SYSCLK 12000000
//#define SYSCLK   33333333
#define   SYSCLK   24997500


#define CLK_PERIOD (1e+9 / SYSCLK)  // in ns...

//#define DCACHE_SIZE 0 // (2048*4)  // DCache Size in Bytes


// Parameters for SPI Flash

#define FLASHSIZE (8192*1024)
#define MAX_FLASH_IMAGESIZE (2024*1024) // Max 2MB of flash used for boot image
#define FLASH_IMAGEBASE (1024*3072)  // Boot Image starts at 3MB in Flash

#define PLATFORM_BAUDRATE 38400

#define NO_SYSCALL
#define NO_FLASH
#define NO_XMODEM
#define NO_DRAMTEST

#define GPIO_TEST


#endif
