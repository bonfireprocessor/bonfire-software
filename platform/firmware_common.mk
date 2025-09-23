# --------------------------------------------
# Common RISC-V firmware make definitions/rules
# --------------------------------------------

#.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.SUFFIXES:

.PHONY: all clean run flash help print-% default

# -------- Defaults (override in including Makefile) --------
ARCH            ?= rv32im_zicsr_zifencei
ABI             ?= ilp32
PLATFORM        ?= BASIC


SOFTROOT        ?= ../
PROJROOT        ?= $(SOFTROOT)/..

PLATFORMDIR     ?= ../platform
LINKDEF         ?= $(PLATFORMDIR)/$(PLATFORM)/firmware.ld



# Toolchain (GNU RISC-V)
TARGET_PREFIX   ?= riscv64-unknown-elf
CC              := $(TARGET_PREFIX)-gcc
LD              := $(TARGET_PREFIX)-gcc
AR              := $(TARGET_PREFIX)-ar
OBJCOPY         := $(TARGET_PREFIX)-objcopy
OBJDUMP         := $(TARGET_PREFIX)-objdump
SIZE            := $(TARGET_PREFIX)-size

HEXDUMP         ?= hexdump

# Project name / artifacts
TARGET_BASE     ?= $(PLATFORM)_monitor

# Build directory (platform-scoped)
BUILD_ROOT      ?= build
BUILD           := $(BUILD_ROOT)/$(PLATFORM)

# Source layout (project may extend SRCDIRS/INCLUDES before including this file)
SRCDIRS         += .
INCLUDES        += -I$(PLATFORMDIR) -I$(PLATFORMDIR)/$(PLATFORM) -I$(SOFTROOT)/riscv  -I.


# --- Normalize source dirs and enforce uniqueness ---
# strip trailing slashes and normalize "./" to "."
SRCDIRS := $(patsubst %/,%,$(SRCDIRS))
SRCDIRS := $(patsubst ./,.,$(SRCDIRS))
# unique dirs
SRCDIRS := $(sort $(SRCDIRS))

# Auto-discover sources
SRC_C           := $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.c))
SRC_S_UPPER     := $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.S))
SRC_S_LOWER     := $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.s))

# Map to objects in BUILD tree (mirror subpaths)
OBJ_C           := $(patsubst %.c,$(BUILD)/%.o,$(SRC_C))
OBJ_S_UPPER     := $(patsubst %.S,$(BUILD)/%.o,$(SRC_S_UPPER))
OBJ_S_LOWER     := $(patsubst %.s,$(BUILD)/%.o,$(SRC_S_LOWER))
OBJECTS         := $(OBJ_C) $(OBJ_S_UPPER) $(OBJ_S_LOWER)

# unique source lists
SRC_C           := $(sort $(SRC_C_RAW))
SRC_S_UPPER     := $(sort $(SRC_S_UPPER_RAW))
SRC_S_LOWER     := $(sort $(SRC_S_LOWER_RAW))

# Dependency files
DEPS            := $(OBJECTS:.o=.d)

# Flags
COMMON_ARCH     := -march=$(ARCH) -mabi=$(ABI)
COMMON_WARN     := -Wall -Wextra -Werror=implicit-function-declaration
COMMON_MISC     := -ffreestanding -fno-builtin -fomit-frame-pointer -mstrict-align
COMMON_OPTS     := -Os -g
DEPFLAGS        := -MMD -MP

CFLAGS          += $(COMMON_ARCH) $(COMMON_WARN) $(COMMON_MISC) $(COMMON_OPTS) $(DEPFLAGS) \
                   -D$(PLATFORM) $(INCLUDES)
ASFLAGS         += $(COMMON_ARCH) -g $(DEPFLAGS) -D$(PLATFORM) $(INCLUDES)
LDFLAGS         += $(COMMON_ARCH) -nostartfiles -L$(PLATFORMDIR) \
                   -Wl,-m,elf32lriscv -Wl,-T$(LINKDEF) -Wl,--gc-sections



# Artifacts
ELF := $(BUILD)/$(TARGET_BASE).elf
BIN := $(BUILD)/$(TARGET_BASE).bin
HEX := $(BUILD)/$(TARGET_BASE).hex
LST := $(BUILD)/$(TARGET_BASE).lst
DMP := $(BUILD)/$(TARGET_BASE).dmp
BIT := $(BUILD)/$(TARGET_BASE).bit

# ---------- Top-Level Targets ----------
default: all
all: $(HEX) $(BIN)

run: $(BIT)
	@echo "[PROG] $<"
	$(PROG) $<

flash: run

help:
	@echo "Targets:"
	@echo "  make            -> build $(HEX) and $(BIN)"
	@echo "  make run        -> program FPGA with $(BIT)"
	@echo "  make clean      -> remove build folder"
	@echo "Variables (override via CLI):"
	@echo "  PLATFORM=$(PLATFORM) ARCH=$(ARCH) ABI=$(ABI)"
	@echo "  TARGET_PREFIX=$(TARGET_PREFIX) TOPLEVEL=$(TOPLEVEL)"
	@echo "  BMMFILE=$(BMMFILE) DATA2MEM=$(DATA2MEM) PROG='$(PROG)'"
	@echo "  V=1 for verbose"

# ---------- Link / Convert / Bitstream ----------
$(ELF): $(OBJECTS) $(LINKDEF) | $(BUILD)
	$(LD) -o $@ $(OBJECTS) $(LDFLAGS)
	$(SIZE) $@

$(BIN): $(ELF)
	$(OBJCOPY) -S -O binary $< $@

$(HEX): $(ELF) $(BIN)
	$(HEXDUMP) -v -e '1/4 "%08x\n"' $(BIN) > $@
	$(OBJDUMP) -S -d $< > $(LST)
	$(OBJDUMP) -s $< > $(DMP)
	



# ---------- Compile Rules ----------
# Compile rules (create output dir per object)
$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

# Root build dir (optional, kept for top-level targets)
$(BUILD):
	mkdir -p $@



# ---------- Cleaning ----------
clean:
	@echo "[CLEAN] $(BUILD_ROOT)"
	rm -rf $(BUILD_ROOT)

# ---------- Debug ----------
print-%:
	@echo '$*=$($*)'

# ---------- Dependencies ----------
-include $(DEPS)