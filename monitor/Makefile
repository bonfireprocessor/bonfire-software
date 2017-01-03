
.PHONY: all clean

ARCH=RV32I
PLATFORM=PAPRO


TARGET_PREFIX ?= riscv32-unknown-elf
TARGET_CC := $(TARGET_PREFIX)-gcc
TARGET_LD := $(TARGET_PREFIX)-gcc
TARGET_SIZE := $(TARGET_PREFIX)-size
TARGET_OBJCOPY := $(TARGET_PREFIX)-objcopy
HEXDUMP ?= hexdump
DATA2MEM = /opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64/data2mem

SOFTROOT = ../..
PROJROOT = $(SOFTROOT)/..

TOPLEVEL = ~/riscv/ise/bonfire/papilio_pro_dram_toplevel.bit
PLATFORMDIR=../platform
LINKDEF=$(PLATFORMDIR)/$(PLATFORM)/firmware.ld



TARGET_CFLAGS += -m32 -march=$(ARCH) -Wall -Og -g  -fomit-frame-pointer \
          -msoft-float\
	-ffreestanding -fno-builtin  \
	-Wall -Werror=implicit-function-declaration \
	-D$(PLATFORM) -I$(PLATFORMDIR) -I$(PLATFORMDIR)/$(PLATFORM)

TARGET_LDFLAGS += -m32 -march=$(ARCH) -nostartfiles  \
	-Wl,-m,elf32lriscv --specs=nosys.specs -Wl,-T$(LINKDEF) \
	-Wl,--gc-sections

all: monitor.hex

%.bit: %.elf
	$(DATA2MEM) -bm $(PROJROOT)/platform/wildfire_papro/soc_bd.bmm  -bd $< -bt $(TOPLEVEL)  -o hb $(basename $@)

%.o : %.S
	$(TARGET_CC) $(TARGET_CFLAGS) -c $<

%.o : %.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c $<



%.hex : %.elf
	$(TARGET_OBJCOPY) -j .text -j .data -O binary $< $(basename $@).bin
	$(HEXDUMP) -v -e '1/4 "%08x\n"' $(basename $@).bin >$@
	$(TARGET_PREFIX)-objdump -S -d $< >$(basename $@).lst
	$(TARGET_PREFIX)-objdump -s $< >$(basename $@).dmp
	$(TARGET_PREFIX)-size  $<


	
monitor.elf : start.o monitor.o uart.o snprintf.o string.o console.o mempattern.o xmodem.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o monitor.o uart.o snprintf.o string.o console.o mempattern.o xmodem.o
	
