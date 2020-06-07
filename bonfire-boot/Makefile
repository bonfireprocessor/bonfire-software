.PHONY: build_picotcp test all clean

BUILD ?=build

all: $(BUILD)/bonfire-boot.bin $(BUILD)/bonfire-boot.elf

PICO_ROOT?=picotcp
PICO_BUILD=$(PICO_ROOT)/build_rv32

TARGET_NAME = bonfire-boot

# SPIFFS
spiffs?=spiffs
include spiffs/files.mk
# the above include will append to CPATH, CFILES FLAGS and INC
SPIFFS_OBJ = $(CFILES:%.c=${BUILD}/%.o)

OBJECTS = $(BUILD)/main.o $(BUILD)/sys.o $(BUILD)/xil_etherlite.o $(BUILD)/ethernet_driver.o \
		  $(BUILD)/tftp.o $(BUILD)/spi.o $(BUILD)/newlib_stubs.o  \
		  $(BUILD)/spiflash.o $(BUILD)/spiffs_hal.o $(BUILD)/shell.o \
		  $(BUILD)/uart.o $(BUILD)/console.o $(BUILD)/monitor.o	$(BUILD)/snprintf.o \
		  $(BUILD)/riscv-gdb-stub.o $(BUILD)/gdb_interface.o $(BUILD)/gdb_traps.o $(BUILD)/trap.o \
		  $(SPIFFS_OBJ)


GDBSTUB = ../gdb-stub

vpath %.c spiflash_driver/src $(CPATH) $(GDBSTUB)
vpath %.S $(GDBSTUB)

include  ../bonfire_defs.Makefile.include

INCLUDES += $(INC) -I. -I$(PICO_BUILD)/include -I$(PICO_BUILD)/include/arch -Ispiflash_driver/src -I$(GDBSTUB)
TARGET_CFLAGS +=  $(FLAGS) -L$(CURDIR)  -DRISCV32
LIBS += -L$(PICO_BUILD)/lib -lpicotcp

PICO_MIN_OPTIONS?= IPFILTER=0 DHCP_SERVER=0 NAT=0 MCAST=0  \
				   SNTP_CLIENT=0 IPFILTER=0 SLAACV4=0 \
	 		       PPP=0 IPV6=0 AODV=0 MDNS=0 TFTP=1 TCP=0 DEBUG=1


build_picotcp:
	cd $(PICO_ROOT) ; make -f riscvcross.mk PICO_OPTIONS="$(PICO_MIN_OPTIONS)" build


clean:
	rm -f $(BUILD)/*

test:
	echo $(OBJECTS)	
	echo $(DEPS)
	echo $(CURDIR)