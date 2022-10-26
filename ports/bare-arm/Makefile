# Include the core environment definitions; this will set $(TOP).
include ../../py/mkenv.mk

# Include py core make definitions.
include $(TOP)/py/py.mk

# Set makefile-level MicroPython feature configurations.
MICROPY_ROM_TEXT_COMPRESSION ?= 0

# Define toolchain and other tools.
CROSS_COMPILE ?= arm-none-eabi-
DFU ?= $(TOP)/tools/dfu.py
PYDFU ?= $(TOP)/tools/pydfu.py

# Set CFLAGS.
CFLAGS += -I. -I$(TOP) -I$(BUILD)
CFLAGS += -Wall -Werror -std=c99 -nostdlib
CFLAGS += -mthumb -mtune=cortex-m4 -mcpu=cortex-m4 -msoft-float
CSUPEROPT = -Os # save some code space for performance-critical code
CFLAGS += -I$(TOP)/lib/cmsis/inc
CFLAGS += -I$(HOME)/others/AmbiqSuite_R4.3.0/CMSIS/AmbiqMicro/Include
CFLAGS += -I$(HOME)/others/AmbiqSuite_R4.3.0/mcu/apollo4p

# Select debugging or optimisation build.
ifeq ($(DEBUG), 1)
CFLAGS += -Og
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

# Set linker flags.
LDFLAGS += -nostdlib -T stm32f405.ld --gc-sections

# Define the required source files.
SRC_C += lib.c main.c system.c
SRC_C += $(HOME)/others/AmbiqSuite_R4.3.0/mcu/apollo4p/hal/am_hal_pwrctrl.c
SRC_C += $(HOME)/others/AmbiqSuite_R4.3.0/mcu/apollo4p/hal/mcu/am_hal_utils.c
SRC_C += $(HOME)/others/AmbiqSuite_R4.3.0/mcu/apollo4p/hal/mcu/am_hal_bootrom_helper.c
SRC_C += $(HOME)/others/AmbiqSuite_R4.3.0/mcu/apollo4p/hal/mcu/am_hal_interrupt.c

# Define the required object files.
OBJ += $(PY_CORE_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

# Define the top-level target, the main firmware.
all: $(BUILD)/firmware.dfu

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^
	$(Q)$(SIZE) $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(ECHO) "Create $@"
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $@

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(ECHO) "Create $@"
	$(Q)$(OBJCOPY) -O ihex $^ $@

$(BUILD)/firmware.dfu: $(BUILD)/firmware.bin
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(DFU) -b 0x08000000:$^ $@

deploy: $(BUILD)/firmware.dfu
	$(Q)$(PYTHON) $(PYDFU) -u $^

JLINK_EXE ?= $(HOME)/others/JLink_Linux_V754c_x86_64/JLinkExe
JLINK_DEV ?= AMAP42KK-KBR
JLINK_CMD = '\
ExitOnError 1\n\
Device $(JLINK_DEV)\n\
SelectInterface SWD\n\
Speed auto\n\
Connect\n\
Reset\n\
LoadFile "$(BUILD)/firmware.hex"\n\
Exit'

deploy-jlink: $(BUILD)/firmware.hex
	$(Q)echo -e $(JLINK_CMD) | $(JLINK_EXE)

# Include remaining core make rules.
include $(TOP)/py/mkrules.mk
