################################################################################
# Initial MicroPython make rules

# Include the core environment definitions; this will set $(TOP).
include ../../py/mkenv.mk

# MicroPython feature configurations.
MICROPY_ROM_TEXT_COMPRESSION ?= 1
MICROPY_VFS_FAT ?= 1
MICROPY_VFS_LFS2 ?= 1
MICROPY_PY_LWIP ?= 1
MICROPY_PY_NETWORK_CYW43 ?= 0

# qstr definitions (must come before including py.mk).
QSTR_DEFS = qstrdefsport.h

# Include py core make definitions.
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

################################################################################
# Port specific compiler/linker flags

CROSS_COMPILE ?= arm-none-eabi-
AMBIQ_SUITE_REL = $(TOP)/lib/ambiq-suite
AMBIQ_SUITE_TOP = lib/ambiq-suite

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

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(TOP)/lib/cmsis/inc
INC += -I$(AMBIQ_SUITE_REL)/CMSIS/AmbiqMicro/Include
INC += -I$(AMBIQ_SUITE_REL)/mcu/apollo4p
INC += -I$(AMBIQ_SUITE_REL)/utils
INC += -Ilwip_inc

CFLAGS += $(INC)
CFLAGS += -mthumb -mtune=cortex-m4 -mcpu=cortex-m4
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -fsingle-precision-constant -Wdouble-promotion -Wfloat-conversion
CFLAGS += -Wall -Werror -std=c99 -nostdlib $(COPT)

LDFLAGS += -nostdlib -T ambiq_apollo4p.ld -Map=$@.map --cref --gc-sections

# Tune for Debugging or Optimization
CFLAGS += -g
ifeq ($(DEBUG), 1)
CFLAGS += -O0
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

# Flags for optional C++ source code
CXXFLAGS += $(filter-out -std=c99,$(CFLAGS))

################################################################################
# Source files and libraries

LIBS += "$(shell $(CC) $(CFLAGS) -print-libgcc-file-name)"

SRC_C += \
	am_bsp.c \
	amap.c \
	fatfs_port.c \
	machine_pin.c \
	main.c \
	modmachine.c \
	modtime.c \
	mphalport.c \
	mpnetworkport.c \
	pin.c \
	sdio.c \
	tusb_port.c \
	uart.c \
	usbd.c \
	dcd_apollo4.c \
	shared/libc/printf.c \
	shared/libc/string0.c \
	shared/netutils/dhcpserver.c \
	shared/netutils/trace.c \
	shared/readline/readline.c \
	shared/runtime/gchelper_native.c \
	shared/runtime/interrupt_char.c \
	shared/runtime/pyexec.c \
	shared/runtime/stdout_helpers.c \
	shared/runtime/sys_stdio_mphal.c \
	shared/timeutils/timeutils.c \
	shared/tinyusb/mp_cdc_common.c \
	shared/tinyusb/mp_usbd.c \
	drivers/bus/softspi.c \

SRC_C += \
	lib/tinyusb/src/class/cdc/cdc_device.c \
	lib/tinyusb/src/common/tusb_fifo.c \
	lib/tinyusb/src/device/usbd.c \
	lib/tinyusb/src/device/usbd_control.c \
	lib/tinyusb/src/tusb.c \

CFLAGS += -I$(TOP)/lib/tinyusb/src

SRC_C += $(addprefix lib/libm/,\
	math.c \
	acoshf.c \
	asinfacosf.c \
	asinhf.c \
	atan2f.c \
	atanf.c \
	atanhf.c \
	ef_rem_pio2.c \
	erf_lgamma.c \
	fmodf.c \
	kf_cos.c \
	kf_rem_pio2.c \
	kf_sin.c \
	kf_tan.c \
	log1pf.c \
	nearbyintf.c \
	roundf.c \
	sf_cos.c \
	sf_erf.c \
	sf_frexp.c \
	sf_ldexp.c \
	sf_modf.c \
	sf_sin.c \
	sf_tan.c \
	thumb_vfp_sqrtf.c \
	wf_lgamma.c \
	wf_tgamma.c \
	)

SRC_S += shared/runtime/gchelper_thumb2.s

SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/am_hal_gpio.c
$(BUILD)/$(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/am_hal_gpio.o: CFLAGS += -Wno-maybe-uninitialized
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/am_hal_pwrctrl.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/am_hal_queue.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/am_hal_usb.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_bootrom_helper.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_cachectrl.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_card.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_card_host.c
$(BUILD)/$(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_card_host.o: CFLAGS += -Wno-maybe-uninitialized
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_clkgen.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_interrupt.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_mram.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_rtc.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_sdhc.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_uart.c
SRC_C += $(AMBIQ_SUITE_TOP)/mcu/apollo4p/hal/mcu/am_hal_utils.c

SRC_QSTR += \
	machine_pin.c \
	main.c \
	modmachine.c \
	modtime.c \
	shared/readline/readline.c \
	shared/runtime/pyexec.c \
	shared/runtime/sys_stdio_mphal.c \

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.s=.o))

################################################################################
# Main targets

.PHONY: all
all: $(BUILD)/firmware.hex

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O ihex $^ $@

.PHONY: deploy-jlink
deploy-jlink: $(BUILD)/firmware.hex
	$(Q)echo -e $(JLINK_CMD) | $(JLINK_EXE)

################################################################################
# Remaining MicroPython make rules

include $(TOP)/py/mkrules.mk
