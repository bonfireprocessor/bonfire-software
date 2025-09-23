# --------------------------------------
# Project Makefile (target-specific bits)
# --------------------------------------
# Override variables here; then include common.mk

# Target / platform specifics
# PLATFORM        ?= BASIC
# ARCH            ?= rv32im_zicsr_zifencei
# ABI             ?= ilp32
# UART            ?= uart

# Naming
TARGET_BASE     ?= $(PLATFORM)_monitor

# Paths
# SOFTROOT        ?= ../
# PROJROOT        ?= $(SOFTROOT)/..
# PLATFORMDIR     ?= ../platform
# LINKDEF         ?= $(PLATFORMDIR)/$(PLATFORM)/firmware.ld



# Sources: extend as needed
SRCDIRS         := . spiflash_driver/src
INCLUDES        := -I./spiflash_driver/src

# Pull in the shared logic
include ../platform/firmware_common.mk