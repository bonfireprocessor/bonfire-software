.PHONY: build_picotcp fuck all clean

PICO_ROOT?=picotcp
PICO_BUILD=$(PICO_ROOT)/build_rv32

TARGET_NAME = bonfire-boot
OBJECTS = $(BUILD)/main.o  $(BUILD)/sys.o  $(BUILD)/xil_etherlite.o $(BUILD)/ethernet_driver.o \
		  $(BUILD)/tftp.o \
		  $(BUILD)/uart.o $(BUILD)/console.o $(BUILD)/monitor.o	$(BUILD)/snprintf.o

include  ../bonfire_defs.Makefile.include

TARGET_CFLAGS += -I$(PICO_BUILD)/include -I$(PICO_BUILD)/include/arch -DRISCV32
LIBS += -L$(PICO_BUILD)/lib -lpicotcp

PICO_MIN_OPTIONS?= IPFILTER=0 DHCP_SERVER=0 NAT=0 MCAST=0  \
				   SNTP_CLIENT=0 IPFILTER=0 SLAACV4=0 \
	 		       PPP=0 IPV6=0 AODV=0 MDNS=0 TFTP=1 TCP=0


build_picotcp:
	cd $(PICO_ROOT) ; make -f riscvcross.mk PICO_OPTIONS="$(PICO_MIN_OPTIONS)" build




all: build/bonfire-boot.bin

fuck: $(OBJECTS)
	echo $(TARGET_CFLAGS)

clean:
	rm -f $(BUILD)/*	