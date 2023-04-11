/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
Python code to test PSRAM:

import machine
BASE = 0x14000000
m32 = lambda addr: machine.mem32[addr] & 0xffffffff
machine.mem32[BASE] = 0xdeadbeef
machine.mem32[BASE + 4] = 0xfacecafe
machine.mem32[BASE + 8] = 0xfeedc0de
machine.mem32[BASE + 12] = 0x12345678
for i in range(256+8):
    offset = i * 128 * 1024
    ii = BASE + offset
    print(f"PSRAM+{offset // 1024:5}k = 0x{ii:08x} : {m32(ii):08x} {m32(ii + 4):08x} {m32(ii + 8):08x} {m32(ii + 12):08x}")

*/

#include "py/mperrno.h"
#include "py/runtime.h"
#include "pins.h"
#include "am_mcu_apollo.h"
#include "am_devices_mspi_psram_aps12808l.h"

#define MSPI_ID (0)
#define MSPI_IRQ (MSPI0_IRQn)

static uint32_t DMATCBBuffer[2560];
static void *g_pDevHandle;
static void *g_pHandle;

static am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

static am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;

#define AMAP_PIN_PINCFG_PSRAM_CE0 \
    { \
        .GP.cfg_b.uFuncSel             = AM_HAL_PIN_57_NCE57, \
        .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE, \
        .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN, \
        .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE, \
        .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL, \
        .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X, \
        .GP.cfg_b.uSlewRate            = 0, \
        .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE, \
        .GP.cfg_b.uNCE                 = AM_HAL_GPIO_NCE_MSPI0CEN0, \
        .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW, \
        .GP.cfg_b.uRsvd_0              = 0, \
        .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE, \
        .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE, \
        .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE, \
        .GP.cfg_b.uRsvd_1              = 0, \
    }

#define AMAP_PIN_PINCFG_PSRAM_Dx \
    { \
        .GP.cfg_b.uFuncSel             = AM_HAL_PIN_64_MSPI0_0, \
        .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE, \
        .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN, \
        .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE, \
        .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE, \
        .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X, \
        .GP.cfg_b.uSlewRate            = 0, \
        .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE, \
        .GP.cfg_b.uNCE                 = 0, \
        .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW, \
        .GP.cfg_b.uRsvd_0              = 0, \
        .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE, \
        .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE, \
        .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE, \
        .GP.cfg_b.uRsvd_1              = 0, \
    }

#define AMAP_PIN_PINCFG_PSRAM_SCK \
    { \
        .GP.cfg_b.uFuncSel             = AM_HAL_PIN_72_MSPI0_8, \
        .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE, \
        .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN, \
        .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE, \
        .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE, \
        .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X, \
        .GP.cfg_b.uSlewRate            = 0, \
        .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE, \
        .GP.cfg_b.uNCE                 = 0, \
        .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW, \
        .GP.cfg_b.uRsvd_0              = 0, \
        .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE, \
        .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE, \
        .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE, \
        .GP.cfg_b.uRsvd_1              = 0, \
    }

#define AMAP_PIN_PINCFG_PSRAM_DQS0DM0 \
    { \
        .GP.cfg_b.uFuncSel             = AM_HAL_PIN_73_MSPI0_9,                 \
        .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,                    \
        .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,                    \
        .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,                   \
        .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,                    \
        .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X,                    \
        .GP.cfg_b.uSlewRate            = 0,                 \
        .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,                   \
        .GP.cfg_b.uNCE                 = 0,                 \
        .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,                   \
        .GP.cfg_b.uRsvd_0              = 0,                 \
        .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,                  \
        .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,                  \
        .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,                  \
        .GP.cfg_b.uRsvd_1              = 0,                 \
    }

// Set up the MSPI pins based on the external flash device type.
void am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice) {
    assert(ui32Module == 0);
    assert(eMSPIDevice == AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_CE0, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_CE0);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D0, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D1, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D2, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D3, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D4, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D5, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D6, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_D7, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_Dx);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_SCK, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_SCK);
    am_hal_gpio_pinconfig(amap_pin_PSRAM_DQS0DM0, (am_hal_gpio_pincfg_t)AMAP_PIN_PINCFG_PSRAM_DQS0DM0);
}

NORETURN void mp_raise_OSError_with_msg(int errno_, const char *msg) {
    vstr_t vstr;
    vstr_init(&vstr, 32);
    vstr_add_str(&vstr, msg);
    mp_obj_t o_str = mp_obj_new_str_from_vstr(&vstr);
    mp_obj_t args[2] = { MP_OBJ_NEW_SMALL_INT(errno_), MP_OBJ_FROM_PTR(o_str)};
    nlr_raise(mp_obj_exception_make_new(&mp_type_OSError, 2, 0, args));
}

STATIC mp_obj_t memtest_psram_timing_check(void) {
    mp_printf(MICROPY_DEBUG_PRINTER, "Starting MSPI DDR Timing Scan: \n");
    uint32_t ret = am_devices_mspi_psram_aps12808l_ddr_init_timing_check(MSPI_ID, &MSPI_PSRAM_OctalCE0MSPIConfig, &MSPIDdrTimingConfig);
    if (ret == AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS) {
        mp_printf(MICROPY_DEBUG_PRINTER, "==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    } else {
        mp_printf(MICROPY_DEBUG_PRINTER, "==== Scan Result: Failed, no valid setting.  \n");
    }
    return MP_OBJ_NEW_SMALL_INT(MSPIDdrTimingConfig.ui32Rxdqsdelay);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(memtest_psram_timing_check_obj, memtest_psram_timing_check);

STATIC mp_obj_t memtest_psram_init(void) {
    if (MSPIDdrTimingConfig.ui32Rxdqsdelay == 0) {
        memtest_psram_timing_check();
    }

    // Configure the MSPI and PSRAM Device.
    uint32_t ret = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_ID, &MSPI_PSRAM_OctalCE0MSPIConfig, &g_pDevHandle, &g_pHandle);
    if (ret != AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS) {
        mp_raise_OSError_with_msg(MP_EIO, "failed to configure the MSPI and PSRAM device");
    }
    NVIC_SetPriority(MSPI_IRQ, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(MSPI_IRQ);

    am_hal_interrupt_master_enable();

    // Set the DDR timing from previous scan.
    ret = am_devices_mspi_psram_aps12808l_apply_ddr_timing(g_pDevHandle, &MSPIDdrTimingConfig);
    if (ret != AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS) {
        mp_raise_OSError_with_msg(MP_EIO, "failed to set timing");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(memtest_psram_init_obj, memtest_psram_init);

STATIC mp_obj_t memtest_psram_xip(mp_obj_t active) {
    uint32_t ret;
    if (mp_obj_is_true(active)) {
        ret = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pDevHandle);
    } else {
        ret = am_devices_mspi_psram_aps12808l_ddr_disable_xip(g_pDevHandle);
    }
    if (ret != AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS) {
        mp_raise_OSError_with_msg(MP_EIO, "failed to configure xip");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(memtest_psram_xip_obj, memtest_psram_xip);

STATIC const mp_rom_map_elem_t memtest_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_memtest) },
    { MP_ROM_QSTR(MP_QSTR_psram_timing_check),  MP_ROM_PTR(&memtest_psram_timing_check_obj) },
    { MP_ROM_QSTR(MP_QSTR_psram_init),          MP_ROM_PTR(&memtest_psram_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_psram_xip),           MP_ROM_PTR(&memtest_psram_xip_obj) },
};
STATIC MP_DEFINE_CONST_DICT(memtest_module_globals, memtest_module_globals_table);

const mp_obj_module_t mp_module_memtest = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&memtest_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_memtest, mp_module_memtest);
