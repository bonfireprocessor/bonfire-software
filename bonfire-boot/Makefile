.PHONY: build_picotcp test all clean

BUILD ?=build

all: $(BUILD)/bonfire-boot.bin $(BUILD)/bonfire-boot.elf

PICO_ROOT?=picotcp
PICO_BUILD=$(PICO_ROOT)/build_rv32

TARGET_NAME = bonfire-boot

LINKDEF_DRAM=bonfire-boot.ld

# SPIFFS
spiffs?=spiffs
include spiffs/files.mk
# the above include will append to CPATH, CFILES FLAGS and INC
SPIFFS_OBJ = $(CFILES:%.c=${BUILD}/%.o)

SRCFILES += main.c sys.c xil_etherlite.c ethernet_driver.c tftp.c spi.c newlib_stubs.c \
		    spiflash.c shell.c uart.c console.c monitor.c snprintf.c \
		    riscv-gdb-stub.c gdb_interface.c trap.c \
			spiffs_hal.c

#LittleFS
SRCFILES += lfs.c lfs_util.c  littlefs_hal.c		  

LOC_OBJ =  $(SRCFILES:%.c=${BUILD}/%.o)

OBJECTS = $(LOC_OBJ) $(SPIFFS_OBJ) $(BUILD)/gdb_traps.o 


GDBSTUB = ../gdb-stub

vpath %.c spiflash_driver/src $(CPATH) $(GDBSTUB) littlefs
vpath %.S $(GDBSTUB)

include  ../bonfire_defs.Makefile.include

INCLUDES += $(INC) -I. -I$(PICO_BUILD)/include -I$(PICO_BUILD)/include/arch -Ispiflash_driver/src -Ilittlefs -I$(GDBSTUB)
TARGET_CFLAGS +=  $(FLAGS) -L$(CURDIR)  -DRISCV32 
#-DLFS_NO_ASSERT
# -DLFS_YES_TRACE

TARGET_LDFLAGS_SYSCALL +=  -Wl,--wrap=printf -Wl,--wrap=_exit  -Wl,--wrap=__assert_func

LIBS += -L$(PICO_BUILD)/lib -lpicotcp

PICO_MIN_OPTIONS?= IPFILTER=0 DHCP_SERVER=0 NAT=0 MCAST=0  \
				   SNTP_CLIENT=0 IPFILTER=0 SLAACV4=0 \
	 		       PPP=0 IPV6=0 AODV=0 MDNS=0 TFTP=1 TCP=0 DEBUG=1


build_picotcp:
	cd $(PICO_ROOT) ; make -f riscvcross.mk PICO_OPTIONS="$(PICO_MIN_OPTIONS)" build


clean:
	rm -f $(BUILD)/*

test:
	echo $(LINKDEF_DRAM)
	echo $(OBJECTS)	
	echo $(DEPS)
	echo $(CURDIR)
	echo $(LOC_OBJ)