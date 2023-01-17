/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Damien P. George
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
#ifndef MICROPY_INCLUDED_AMBIQ_APOLLO4_TUSB_CONFIG_H
#define MICROPY_INCLUDED_AMBIQ_APOLLO4_TUSB_CONFIG_H

// Common configuration

// Needed for dcd_apollo4.c
//#define BOARD_DEVICE_RHPORT_SPEED   OPT_MODE_FULL_SPEED
#define BOARD_DEVICE_RHPORT_SPEED   OPT_MODE_HIGH_SPEED

#define CFG_TUSB_OS                 (OPT_OS_NONE)
#define CFG_TUSB_MCU                OPT_MCU_APOLLO4
#define CFG_TUSB_RHPORT0_MODE       (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN          TU_ATTR_ALIGNED(4)

// Device configuration

#define CFG_TUD_ENDPOINT0_SIZE      (64)
#define TUP_DCD_ENDPOINT_MAX        (6)
#define CFG_TUD_CDC                 (1)
#define CFG_TUD_CDC_RX_BUFSIZE      (512)
#define CFG_TUD_CDC_TX_BUFSIZE      (512)

//#define CFG_TUSB_DEBUG 3

#include "py/mphal.h"
static inline int usbd_ticks(void) {
    return mp_hal_ticks_ms();
}

#endif // MICROPY_INCLUDED_AMBIQ_APOLLO4_TUSB_CONFIG_H
