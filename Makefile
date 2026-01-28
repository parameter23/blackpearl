###############################################################################
# Projektname
# Der Name bestimmt die erzeugten Dateien:
#   console.elf, console.bin, console.map
###############################################################################
PROJECT = blackpearl

###############################################################################
# Quellcode-Dateien
# Alle .c-Dateien im aktuellen Verzeichnis werden automatisch eingebunden.
###############################################################################
CFILES = $(wildcard *.c)

# Objektdateien erzeugen (.o)
# main.o wird ausgeschlossen, falls ein Bootloader oder spezieller Link nötig ist.
OBJS = $(filter-out main.o,$(patsubst %.c,%.o,$(CFILES)))

###############################################################################
# Ziel-MCU
# DEVICE bestimmt das Linkerscript und Startup-Code aus libopencm3.
###############################################################################
DEVICE = stm32f411ce
CFLAGS += -DSTM32F4

###############################################################################
# Architektur- und Floating-Point-Einstellungen
# Für STM32F411 (Cortex-M4F) ist Hardware-FPU korrekt.
###############################################################################
FP_FLAGS   ?= -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS  = -mthumb -mcpu=cortex-m4 $(FP_FLAGS)

# Alternative für Cortex-M3 ohne FPU:
#FP_FLAGS   ?= -msoft-float
#ARCH_FLAGS  = -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

###############################################################################
# OpenOCD Einstellungen
# Diese Werte bestimmen, wie "make flash" den Controller programmiert.
###############################################################################
OOCD            ?= openocd
OOCD_INTERFACE  ?= cmsis-dap
OOCD_TARGET     ?= stm32f4x

###############################################################################
# Include-Pfade
# SHARED_DIR kann zusätzliche Header-Verzeichnisse enthalten.
###############################################################################
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))

###############################################################################
# Pfad zur libopencm3
# Erwartet wird:
#   ./libopencm3/lib/libopencm3_stm32f4.a
###############################################################################
OPENCM3_DIR = ./libopencm3

###############################################################################
# Einbinden der libopencm3-Buildregeln
# Diese erzeugen:
#   - Linkerscript
#   - Startup-Code
#   - Regeln für ELF/BIN/MAP
###############################################################################
include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

###############################################################################
# Benutzerfreundliche Targets
###############################################################################

## make clean – löscht alle erzeugten Dateien
#clean:
#	rm -f *.o *.d *.elf *.bin *.map

# make flash – programmiert den STM32 über OpenOCD
flash: $(PROJECT).bin
	$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
			-f target/$(OOCD_TARGET).cfg \
			-c "program $(PROJECT).bin verify reset exit"

# make size – zeigt die Speicherbelegung
size: $(PROJECT).elf
	arm-none-eabi-size $(PROJECT).elf

###############################################################################
# Ende des Makefiles
###############################################################################
