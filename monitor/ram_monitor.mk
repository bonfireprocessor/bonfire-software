
.PHONY: all clean run

ARCH ?= rv32im
ABI=ilp32
PLATFORM ?= ARTY_AXI

DEFINES = -DGDBSTUB=1 -DBONFIRE -DNO_SYSCALL #-DDEBUG

UART?=uart
TARGETDIR ?= ~/upload

TARGET_PREFIX ?= riscv32-unknown-elf
TARGET_CC := $(TARGET_PREFIX)-gcc
TARGET_LD := $(TARGET_PREFIX)-gcc
TARGET_SIZE := $(TARGET_PREFIX)-size
TARGET_OBJCOPY := $(TARGET_PREFIX)-objcopy
HEXDUMP ?= hexdump
DATA2MEM ?= /opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64/data2mem
PROG?=papilio-prog -v -f

SOFTROOT = ../..
PROJROOT = $(SOFTROOT)/..


PLATFORMDIR=../platform
LINKDEF=./ram_monitor.ld

TARGET_NAME=$(PLATFORM)ram_monitor

GDBSTUB=../gdb-stub 

TARGET_CFLAGS += -march=$(ARCH) -mabi=$(ABI)   -Wall -Og -g -fomit-frame-pointer \
	-ffreestanding -fno-builtin  -mstrict-align \
	-Wall -Werror=implicit-function-declaration -Werror=int-conversion \
	-D$(PLATFORM) $(DEFINES) \
	-I$(PLATFORMDIR) -I$(PLATFORMDIR)/$(PLATFORM) -I$(GDBSTUB)  -I./spiflash_driver/src -I../riscv  -I.

TARGET_LDFLAGS += -march=$(ARCH) -mabi=$(ABI) -nostartfiles  \
	-Wl,-m,elf32lriscv --specs=nano.specs -Wl,-T$(LINKDEF) \
	-Wl,--gc-sections



OBJECTS = start.o monitor.o $(UART).o snprintf.o string.o console.o mempattern.o xm.o spiflash.o syscall.o spi_driver.o testdcache.o

include ../gdb-stub/Makefile.include

vpath %.c spiflash_driver/src:$(GDBSTUB) 

%.bin : $(TARGET_NAME).elf
		$(TARGET_PREFIX)-objcopy -S -O binary $<  $@
		
		
%.lst : %.elf
	$(TARGET_PREFIX)-objdump -S -d $< >$(basename $@).lst
	$(TARGET_PREFIX)-objdump -s $< >$(basename $@).dmp
	$(TARGET_PREFIX)-size  $<		

%.o : %.S
	$(TARGET_CC) $(TARGET_CFLAGS) -c $<

%.o : %.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c $<



%.hex : %.elf
	$(TARGET_OBJCOPY) -S -O binary  $< $(basename $@).bin
	$(HEXDUMP) -v -e '1/4 "%08x\n"' $(basename $@).bin >$@
	$(TARGET_PREFIX)-objdump -S -d $< >$(basename $@).lst
	$(TARGET_PREFIX)-objdump -s $< >$(basename $@).dmp
	$(TARGET_PREFIX)-size  $<



$(TARGET_NAME).elf : $(OBJECTS) $(LINKDEF)
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS)  $(OBJECTS)

bin : $(TARGETDIR)/$(TARGET_NAME).bin

build: $(TARGET_NAME).elf

all: $(TARGETDIR)/$(TARGET_NAME).bin $(TARGET_NAME).lst
#	$(MAKE) -f ram_monitor.mk bin
#	$(MAKE) -f ram_monitor.mk $(TARGET_NAME).lst
