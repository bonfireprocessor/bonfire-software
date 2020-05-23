.PHONY: picotcp

INCLUDE  ../bonfire_defs.Makefile.include


PICO_MIN_OPTIONS?= IPFILTER=0 DHCP_SERVER=0 NAT=0 MCAST=0  \
				   SNTP_CLIENT=0 IPFILTER=0 SLAACV4=0	\
	 		       PPP=0 IPV6=0 AODV=0 MDNS=0 TFTP=1 TCP=0

picotcp:
	cd ./picotcp; \
	make -f riscvcross.mk PICO_OPTIONS="$(PICO_MIN_OPTIONS)" build
