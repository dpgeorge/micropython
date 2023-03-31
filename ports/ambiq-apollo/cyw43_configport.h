/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Damien P. George
 * Copyright (c) 2022 Jim Mussared
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
#ifndef MICROPY_INCLUDED_AMBIQ_APOLLO_CYW43_CONFIGPORT_H
#define MICROPY_INCLUDED_AMBIQ_APOLLO_CYW43_CONFIGPORT_H

#include <stdio.h>
// The board-level config will be included here, so it can set some CYW43 values.
#include "py/mpconfig.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "extmod/modnetwork.h"
//#include "pendsv.h"
#include "pins.h"
#include "sdio.h"

#define CYW43_USE_SPI                   (0)
#define CYW43_LWIP                      (1)
#define CYW43_USE_STATS                 (0)

#ifndef CYW43_CHIPSET_FIRMWARE_INCLUDE_FILE
#define CYW43_CHIPSET_FIRMWARE_INCLUDE_FILE "lib/cyw43-driver/firmware/w4343WA1_7_45_98_50_combined.h"
#endif

#ifndef CYW43_WIFI_NVRAM_INCLUDE_FILE
#define CYW43_WIFI_NVRAM_INCLUDE_FILE   "lib/cyw43-driver/firmware/wifi_nvram_1dx.h"
#endif

#define CYW43_IOCTL_TIMEOUT_US          (1000000)
#define CYW43_SLEEP_MAX                 (50)
#define CYW43_NETUTILS                  (1)
#define CYW43_CLEAR_SDIO_INT            (1)

#define CYW43_EPERM                     MP_EPERM // Operation not permitted
#define CYW43_EIO                       MP_EIO // I/O error
#define CYW43_EINVAL                    MP_EINVAL // Invalid argument
#define CYW43_ETIMEDOUT                 MP_ETIMEDOUT // Connection timed out

#define CYW43_THREAD_ENTER              MICROPY_PY_LWIP_ENTER
#define CYW43_THREAD_EXIT               MICROPY_PY_LWIP_EXIT
#define CYW43_THREAD_LOCK_CHECK

#define CYW43_HOST_NAME                 mod_network_hostname

#define CYW43_SDPCM_SEND_COMMON_WAIT    __WFE();
#define CYW43_DO_IOCTL_WAIT             __WFE();
#define CYW43_EVENT_POLL_HOOK           MICROPY_EVENT_POLL_HOOK

#define CYW43_ARRAY_SIZE(a)             MP_ARRAY_SIZE(a)

#define CYW43_HAL_PIN_MODE_INPUT        MP_HAL_PIN_MODE_INPUT
#define CYW43_HAL_PIN_MODE_OUTPUT       MP_HAL_PIN_MODE_OUTPUT
#define CYW43_HAL_PIN_PULL_NONE         MP_HAL_PIN_PULL_NONE
#define CYW43_HAL_MAC_WLAN0             (0)

#define CYW43_PIN_WL_REG_ON             amap_pin_WL_REG_ON
#define CYW43_PIN_WL_HOST_WAKE          amap_pin_WL_HOST_WAKE
#define CYW43_PIN_WL_RFSW_VDD           amap_pin_WL_RFSW_VDD

static inline unsigned int cyw43_hal_ticks_us(void) {
    return mp_hal_ticks_us();
}

static inline unsigned int cyw43_hal_ticks_ms(void) {
    return mp_hal_ticks_ms();
}

static inline void cyw43_delay_us(unsigned int us) {
    uint32_t start = mp_hal_ticks_us();
    while (mp_hal_ticks_us() - start < us) {
    }
}

static inline void cyw43_delay_ms(unsigned int ms) {
    uint32_t us = ms * 1000;
    uint32_t start = mp_hal_ticks_us();
    while (mp_hal_ticks_us() - start < us) {
        MICROPY_EVENT_POLL_HOOK;
    }
}

static inline void cyw43_hal_get_mac(int interface, uint8_t mac[6]) {
}

static inline void cyw43_hal_pin_config(int pin, int mode, int pull, int alt) {
    printf("cyw43_hal_pin_config %d %d\n", pin, mode);
    if (mode == CYW43_HAL_PIN_MODE_INPUT) {
        mp_hal_pin_input(pin);
    } else if (mode == CYW43_HAL_PIN_MODE_OUTPUT) {
        mp_hal_pin_output(pin);
    }
}

static inline void cyw43_hal_pin_config_irq_falling(int pin) {
}

static inline int cyw43_hal_pin_read(int pin) {
    return mp_hal_pin_read(pin);
}

static inline void cyw43_hal_pin_low(int pin) {
    printf("cyw43_hal_pin_low %d\n", pin);
    mp_hal_pin_low(pin);
}

static inline void cyw43_hal_pin_high(int pin) {
    printf("cyw43_hal_pin_high %d\n", pin);
    mp_hal_pin_high(pin);
}

static inline void cyw43_schedule_internal_poll_dispatch(void (*func)(void)) {
    //pendsv_schedule_dispatch(PENDSV_DISPATCH_CYW43, func)
}

//void cyw43_post_poll_hook(void);

static inline void cyw43_sdio_init(void) {
    sdio_init();
}

static inline void cyw43_sdio_reinit(void) {
    sdio_reenable();
}

static inline void cyw43_sdio_deinit(void) {
    sdio_deinit();
}

static inline void cyw43_sdio_set_irq(bool enable) {
    sdio_enable_irq(enable);
}

static inline void cyw43_sdio_enable_high_speed_4bit(void) {
    sdio_enable_high_speed_4bit();
}

static inline int cyw43_sdio_transfer(uint32_t cmd, uint32_t arg, uint32_t *resp) {
    return sdio_transfer(cmd, arg, resp);
}

static inline int cyw43_sdio_transfer_cmd53(bool write, uint32_t block_size, uint32_t arg, size_t len, uint8_t *buf) {
    return sdio_transfer_cmd53(write, block_size, arg, len, buf);
}

#endif // MICROPY_INCLUDED_AMBIQ_APOLLO_CYW43_CONFIGPORT_H
