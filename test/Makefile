
.PHONY: all clean

ARCH ?= rv32im
ABI=ilp32
PLATFORM ?= PAPRO

BOOTLINK=wildfboot.ld




TARGET_PREFIX ?= riscv32-unknown-elf
TARGET_CC := $(TARGET_PREFIX)-gcc
TARGET_LD := $(TARGET_PREFIX)-gcc
TARGET_SIZE := $(TARGET_PREFIX)-size
TARGET_OBJCOPY := $(TARGET_PREFIX)-objcopy
HEXDUMP ?= hexdump
DATA2MEM = /opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64/data2mem

PROJROOT = ../../..
TOPLEVEL = ~/riscv/ise/bonfire/papilio_pro_dram_toplevel.bit
PLATFORMDIR=../platform

LINKDEF?=$(PLATFORMDIR)/$(PLATFORM)/firmware.ld


TARGET_CFLAGS +=  -march=$(ARCH) -mabi=$(ABI) -Wall -Os -g  -fomit-frame-pointer \
	-ffreestanding -fno-builtin  \
	-Wall -Werror=implicit-function-declaration \
	-D$(PLATFORM) -I$(PLATFORMDIR) -I$(PLATFORMDIR)/$(PLATFORM) -I../riscv

TARGET_LDFLAGS += -march=$(ARCH) -mabi=$(ABI) -nostartfiles   \
	-Wl,-m,elf32lriscv --specs=nosys.specs -Wl,-T$(LINKDEF) \
	-Wl,--gc-sections



all: ledslow.hex counter.hex memtest.hex ledsim.hex mult.hex memsim.hex sim_hello.hex dram_codesim.hex hw_hello.hex minimal.hex

%.bit: %.elf
	$(DATA2MEM) -bm $(PROJROOT)/platform/wildfire_papro/soc_bd.bmm  -bd $< -bt $(TOPLEVEL)  -o hb $(basename $@)

%.o : %.S
	$(TARGET_CC) $(TARGET_CFLAGS)  -c $<

%.o : %.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c $<



%.hex : %.elf
	$(TARGET_OBJCOPY) -S -O binary  $< $(basename $@).bin
	$(HEXDUMP) -v -e '1/4 "%08x\n"' $(basename $@).bin >$@
	$(TARGET_PREFIX)-objdump -S -d $< >$(basename $@).lst
	$(TARGET_PREFIX)-objdump -s $< >$(basename $@).dmp
	$(TARGET_PREFIX)-size  $<


hw_uart.o: uart.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o hw_uart.o -c uart.c

uart.o: uart.c
	$(TARGET_CC) $(TARGET_CFLAGS) -DSIM -c uart.c 	


ledslow.elf: start.o ledslow.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o ledslow.o

ledsim.elf: start.o ledsim.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o ledsim.o


counter.elf: start.o uart.o counter.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o counter.o uart.o

memtest.elf: start.o memtest.o uart.o mempattern.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o memtest.o uart.o mempattern.o

mult.elf : start.o mult.o uart.o snprintf.o console.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o mult.o uart.o snprintf.o console.o

memsim.elf: start.o memsim.o  mempattern.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o memsim.o  mempattern.o

sim_hello.o : sim_hello.c
	$(TARGET_CC) $(TARGET_CFLAGS) -DSIM  -c sim_hello.c 

sim_hello.elf: start.o uart.o sim_hello.o snprintf.o console.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o uart.o sim_hello.o snprintf.o console.o

hw_hello.o : sim_hello.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o hw_hello.o -c sim_hello.c 


hw_hello.elf: start.o hw_uart.o hw_hello.o snprintf.o console.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o hw_uart.o hw_hello.o snprintf.o console.o	

dram_codesim.elf : start.o dram_codesim.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) start.o dram_codesim.o

jump0.elf : jump0.S
	$(TARGET_LD) -o $@ -march=RV32IM -nostartfiles   -Wl,-T$(BOOTLINK)	$<

minimal.elf: minimal.o
	$(TARGET_LD) -o $@ $(TARGET_LDFLAGS) -DSIM  minimal.o

