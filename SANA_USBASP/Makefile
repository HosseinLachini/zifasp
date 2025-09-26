## =========================================================================
## Makefile for AVR project (friendly help + robust targets)
## Project: SANA_UsbAsp
## MCU: atmega88, F_CPU: 16 MHz
## Toolchain: avr-gcc / avr-libc
## Outputs in ../build/<PROJECT>: ELF, HEX, BIN, MAP (+ ZIP/TAR.GZ)
## =========================================================================

## ------------------------ User-configurable ------------------------------
PROJECT   := SANA_UsbAsp
MCU       := atmega88
F_CPU     := 16000000UL
CPPDEFS   :=                 # e.g. -DUSBASP
OPT       := -Os             # optimize for size (typical for AVR)

# Optional prefix if toolchain is in a custom directory (e.g., WinAVR)
# Example: make AVR_PREFIX="C:/WinAVR-20100110/bin/"
AVR_PREFIX ?=

## ------------------------ Paths & tools ----------------------------------
# Build OUTSIDE the source tree:
#   OUT_BASE is where all projects will drop their build dirs.
#   BUILD_DIR becomes <OUT_BASE>/<PROJECT>
OUT_BASE  ?= ../build
BUILD_DIR := $(abspath $(OUT_BASE))/$(PROJECT)
OBJ_DIR   := $(BUILD_DIR)/obj

CC        := $(AVR_PREFIX)avr-gcc
AS        := $(AVR_PREFIX)avr-gcc
LD        := $(AVR_PREFIX)avr-gcc
OBJCOPY   := $(AVR_PREFIX)avr-objcopy
SIZE      := $(AVR_PREFIX)avr-size
AVRDUDE   := $(AVR_PREFIX)avrdude
OBJDUMP   := $(AVR_PREFIX)avr-objdump

# Portable shell commands
ifeq ($(OS),Windows_NT)
  MKDIR := mkdir
  RM    := del /Q
  RMDIR := rmdir /S /Q
else
  MKDIR := mkdir -p
  RM    := rm -f
  RMDIR := rm -rf
endif

## ------------------------ Sources & includes -----------------------------
# Compile units
SOURCES := \
  I2C/I2C.c \
  Zif_Socket/zif.c \
  clock.c \
  fuse.c \
  isp.c \
  main.c \
  tpi.S \
  usbdrv/usbdrv.c \
  usbdrv/usbdrvasm.S \

# Include paths
INCLUDES := \
  -II2C \
  -IZif_Socket \
  -Iusbdrv \

# Objects mirror source tree under $(OBJ_DIR)
OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$(SOURCES))) \
           $(patsubst %.S,$(OBJ_DIR)/%.o,$(filter %.S,$(SOURCES)))
DEPS    := $(OBJECTS:.o=.d)

# Prevent auto-generated .d files from hitting .DEFAULT
$(DEPS): ;

## ------------------------ Flags ------------------------------------------
CFLAGS  := $(OPT) -mmcu=$(MCU) -std=gnu99 -Wall -Wextra -Wundef -Wstrict-prototypes \
           -fdata-sections -ffunction-sections -DF_CPU=$(F_CPU) $(CPPDEFS) $(INCLUDES) -MMD -MP
ASFLAGS := $(OPT) -mmcu=$(MCU) -x assembler-with-cpp -DF_CPU=$(F_CPU) $(CPPDEFS) $(INCLUDES) -MMD -MP
LDFLAGS := -mmcu=$(MCU) -Wl,-Map,$(BUILD_DIR)/$(PROJECT).map -Wl,--gc-sections

ELF := $(BUILD_DIR)/$(PROJECT).elf
HEX := $(BUILD_DIR)/$(PROJECT).hex
BIN := $(BUILD_DIR)/$(PROJECT).bin

## ------------------------ Toolchain preflight ----------------------------
.PHONY: toolchain-check
toolchain-check:
	@command -v "$(CC)"      >/dev/null 2>&1 || { echo "ERROR: avr-gcc not found."; exit 127; }
	@command -v "$(OBJCOPY)" >/dev/null 2>&1 || { echo "ERROR: avr-objcopy not found."; exit 127; }
	@command -v "$(SIZE)"    >/dev/null 2>&1 || { echo "ERROR: avr-size not found."; exit 127; }

## ------------------------ Friendly help & defaults -----------------------
.DEFAULT_GOAL := help

.PHONY: help usage
help usage:
	@echo "AVR Makefile for $(PROJECT)"
	@echo "Usage:"
	@echo "  make                   -> same as 'make help'"
	@echo "  make all               -> build ELF, HEX, BIN into $(BUILD_DIR)"
	@echo "  make main.hex          -> ensure $(PROJECT).hex exists"
	@echo "  make clean             -> remove build artifacts (in $(BUILD_DIR))"
	@echo "  make zip | tar         -> package build outputs"
	@echo "  make disasm            -> disassemble ELF to .lst"
	@echo "  make flash             -> program flash with avrdude (uses HEX)"
	@echo "  make verify            -> verify flash against HEX"
	@echo "  make erase             -> chip erase"
	@echo "  make read-flash        -> read device flash to HEX"
	@echo "  make fuses-read        -> read L/H/E fuses"
	@echo "  make fuses-write LFUSE=0x.. HFUSE=0x.. [EFUSE=0x..]"
	@echo ""
	@echo "Current values:"
	@echo "  PROJECT=$(PROJECT)"
	@echo "  MCU=$(MCU)"
	@echo "  F_CPU=$(F_CPU)"
	@echo "  OPT=$(OPT)"
	@echo "  AVR_PREFIX=$(AVR_PREFIX)"
	@echo "  OUT_BASE=$(OUT_BASE)"
	@echo "  BUILD_DIR=$(BUILD_DIR)"
	@echo "  MCU_ID=$(MCU_ID)"
	@echo "  PROGRAMMER=$(PROGRAMMER)"
	@echo "  PORT=$(PORT)"
	@echo "  BAUD=$(BAUD)"
	@echo "  LFUSE=$(LFUSE)"
	@echo "  HFUSE=$(HFUSE)"
	@echo "  EFUSE=$(EFUSE)"
	@echo ""
	@echo "Tips:"
	@echo "  - Override OUT_BASE to redirect outputs, e.g.:"
	@echo "      make OUT_BASE=/abs/path/to/build"
	@echo "  - Override other vars per your setup, e.g.:"
	@echo "      make MCU=atmega88 MCU_ID=m88 PROGRAMMER=usbasp"

# Unknown target handler: print help and fail (useful for typos).
.DEFAULT:
	@case "$@" in \
	  *.d) exit 0 ;; \
	  *) echo "Unknown target: '$@'"; $(MAKE) --no-print-directory help; false ;; \
	esac

## ------------------------ Phony targets ----------------------------------
.PHONY: all clean size package zip tar \
        flash verify erase read-flash fuses-read fuses-write check-fuses program \
        disasm main.hex

all: toolchain-check $(BUILD_DIR) $(ELF) $(HEX) $(BIN) size package

$(BUILD_DIR):
	$(MKDIR) "$(BUILD_DIR)"
	$(MKDIR) "$(OBJ_DIR)"

## ------------------------ Build rules ------------------------------------
$(ELF): $(OBJECTS)
	@echo "Linking $@"
	@$(LD) $(LDFLAGS) -o "$@" $(OBJECTS)

$(HEX): $(ELF)
	@echo "Generating HEX $@"
	@$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

$(BIN): $(ELF)
	@echo "Generating BIN $@"
	@$(OBJCOPY) -O binary -R .eeprom "$<" "$@"

size: $(ELF)
	@$(SIZE) --mcu=$(MCU) --format=avr "$(ELF)" || $(SIZE) "$(ELF)"

# Ensure subdirs exist and compile
$(OBJ_DIR)/%.o: %.c
	@$(MKDIR) "$(dir $@)"
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/%.o: %.S
	@$(MKDIR) "$(dir $@)"
	@echo "Assembling $<"
	@$(AS) $(ASFLAGS) -c "$<" -o "$@"

# Convenience alias: make main.hex
main.hex: $(HEX)
	@echo "OK: $< is ready."

# Disassemble ELF to human-readable listing
disasm: $(ELF)
	@echo "Generating disassembly -> $(BUILD_DIR)/$(PROJECT).lst"
	@$(OBJDUMP) -d "$(ELF)" > "$(BUILD_DIR)/$(PROJECT).lst"
	@echo "Done: $(BUILD_DIR)/$(PROJECT).lst"

## ------------------------ Packaging --------------------------------------
PKG_FILES := $(HEX) $(BIN) $(ELF) $(BUILD_DIR)/$(PROJECT).map
ZIP := $(BUILD_DIR)/$(PROJECT).zip
TGZ := $(BUILD_DIR)/$(PROJECT).tar.gz

package: zip tar
zip: $(ZIP)
tar: $(TGZ)

ifeq ($(OS),Windows_NT)
$(ZIP): $(PKG_FILES)
	@echo "Creating ZIP (PowerShell) $@"
	@powershell -NoProfile -Command "Compress-Archive -Force -Path $(subst /,\\,$(HEX)),$(subst /,\\,$(BIN)),$(subst /,\\,$(ELF)),$(subst /,\\,$(BUILD_DIR)/$(PROJECT).map) -DestinationPath $(subst /,\\,$(ZIP))"
else
$(ZIP): $(PKG_FILES)
	@echo "Creating ZIP $@"
	@zip -j -9 "$@" $(PKG_FILES)

$(TGZ): $(PKG_FILES)
	@echo "Creating TAR.GZ $@"
	@tar -czf "$@" -C "$(BUILD_DIR)" "$(PROJECT).hex" "$(PROJECT).bin" "$(PROJECT).elf" "$(PROJECT).map"
endif

## ------------------------ avrdude programming ----------------------------
# Device/programmer parameters (can be overridden on the command line)
MCU_ID        ?= m88          # atmega88 device id for avrdude (-p)
PROGRAMMER    ?= usbasp       # change if you use a different programmer
PORT          ?=              # e.g., COM3 or /dev/ttyUSB0 (often not needed for usbasp)
BAUD          ?=

AVRDUDE_FLAGS := -p $(MCU_ID) -c $(PROGRAMMER)
ifneq ($(PORT),)
  AVRDUDE_FLAGS += -P $(PORT)
endif
ifneq ($(BAUD),)
  AVRDUDE_FLAGS += -b $(BAUD)
endif

program: flash

flash: $(HEX)
	@echo "Flashing $(HEX) to $(MCU_ID) via $(PROGRAMMER)"
	@$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:"$<":i
	@echo "Flash done."

verify: $(HEX)
	@echo "Verifying flash against $(HEX)"
	@$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:v:"$<":i

erase:
	@echo "Chip erase"
	@$(AVRDUDE) $(AVRDUDE_FLAGS) -e

read-flash:
	@echo "Reading flash to $(BUILD_DIR)/$(PROJECT)_readback.hex"
	@$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:"$(BUILD_DIR)/$(PROJECT)_readback.hex":i

# Fuse helpers (DANGER: wrong values can brick the MCU)
LFUSE ?=
HFUSE ?=
EFUSE ?=

fuses-read:
	@echo "Reading fuses (hex):"
	@$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

fuses-write: check-fuses
	@echo "Programming fuses: LFUSE=$(LFUSE) HFUSE=$(HFUSE) EFUSE=$(EFUSE)"
	@$(AVRDUDE) $(AVRDUDE_FLAGS) $(if $(LFUSE),-U lfuse:w:$(LFUSE):m) \
	                              $(if $(HFUSE),-U hfuse:w:$(HFUSE):m) \
	                              $(if $(EFUSE),-U efuse:w:$(EFUSE):m)

check-fuses:
ifndef LFUSE
	$(error Please provide LFUSE=0x.. and HFUSE=0x.. (optional EFUSE=0x..). Example: make fuses-write LFUSE=0xFF HFUSE=0xDF EFUSE=0xF9)
endif
ifndef HFUSE
	$(error Please provide LFUSE=0x.. and HFUSE=0x.. (optional EFUSE=0x..). Example: make fuses-write LFUSE=0xFF HFUSE=0xDF EFUSE=0xF9)
endif
	@true

## ------------------------ Cleanup ----------------------------------------
clean:
	@echo "Cleaning build directory: $(BUILD_DIR)"
	-$(RMDIR) "$(BUILD_DIR)"

## ------------------------ Dependencies -----------------------------------
-include $(DEPS)

