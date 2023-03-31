/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2023 Damien P. George
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
#ifndef MICROPY_INCLUDED_AMBIQ_APOLLO_IRQ_H
#define MICROPY_INCLUDED_AMBIQ_APOLLO_IRQ_H

#include "am_mcu_apollo.h"

// Use this macro together with NVIC_SetPriority to indicate that an IRQn is non-negative,
// which helps the compiler optimise the resulting inline function.
#define IRQn_NONNEG(pri) ((pri) & 0x7f)

// these states correspond to values from query_irq, enable_irq and disable_irq
#define IRQ_STATE_DISABLED (0x00000001)
#define IRQ_STATE_ENABLED  (0x00000000)

// 3 bits for pre-emption priority.
#define IRQ_PRI_GROUP (2)

#define IRQ_PRI_SYSTICK         NVIC_EncodePriority(IRQ_PRI_GROUP, 0, 0)

// The UARTs have no FIFOs, so if they don't get serviced quickly then characters
// get dropped. The handling for each character only consumes about 0.5 usec
#define IRQ_PRI_UART            NVIC_EncodePriority(IRQ_PRI_GROUP, 1, 0)

// SDIO must be higher priority than DMA for SDIO DMA transfers to work.
#define IRQ_PRI_SDIO            NVIC_EncodePriority(IRQ_PRI_GROUP, 4, 0)

// DMA should be higher priority than USB, since USB Mass Storage calls
// into the sdcard driver which waits for the DMA to complete.
#define IRQ_PRI_DMA             NVIC_EncodePriority(IRQ_PRI_GROUP, 5, 0)

// Flash IRQ (used for flushing storage cache) must be at the same priority as
// the USB IRQs, so that the IRQ priority can be raised to this level to disable
// both the USB and cache flushing, when storage transfers are in progress.
#define IRQ_PRI_FLASH           NVIC_EncodePriority(IRQ_PRI_GROUP, 6, 0)

#define IRQ_PRI_OTG_FS          NVIC_EncodePriority(IRQ_PRI_GROUP, 6, 0)
#define IRQ_PRI_OTG_HS          NVIC_EncodePriority(IRQ_PRI_GROUP, 6, 0)
#define IRQ_PRI_TIM5            NVIC_EncodePriority(IRQ_PRI_GROUP, 6, 0)

#define IRQ_PRI_CAN             NVIC_EncodePriority(IRQ_PRI_GROUP, 7, 0)

#define IRQ_PRI_SPI             NVIC_EncodePriority(IRQ_PRI_GROUP, 8, 0)

// Interrupt priority for non-special timers.
#define IRQ_PRI_TIMX            NVIC_EncodePriority(IRQ_PRI_GROUP, 13, 0)

#define IRQ_PRI_EXTINT          NVIC_EncodePriority(IRQ_PRI_GROUP, 14, 0)

// PENDSV should be at the lowst priority so that other interrupts complete
// before exception is raised.
#define IRQ_PRI_PENDSV          NVIC_EncodePriority(IRQ_PRI_GROUP, 15, 0)
#define IRQ_PRI_RTC_WKUP        NVIC_EncodePriority(IRQ_PRI_GROUP, 15, 0)

static inline uint32_t query_irq(void) {
    return __get_PRIMASK();
}

static inline void enable_irq(unsigned int state) {
    __set_PRIMASK(state);
}

static inline unsigned int disable_irq(void) {
    unsigned int state = __get_PRIMASK();
    __disable_irq();
    return state;
}

// irqs with a priority value greater or equal to "pri" will be disabled
// "pri" should be between 1 and 15 inclusive
static inline uint32_t raise_irq_pri(uint32_t pri) {
    uint32_t basepri = __get_BASEPRI();
    // If non-zero, the processor does not process any exception with a
    // priority value greater than or equal to BASEPRI.
    // When writing to BASEPRI_MAX the write goes to BASEPRI only if either:
    //   - Rn is non-zero and the current BASEPRI value is 0
    //   - Rn is non-zero and less than the current BASEPRI value
    pri <<= (8 - __NVIC_PRIO_BITS);
    __ASM volatile ("msr basepri_max, %0" : : "r" (pri) : "memory");
    return basepri;
}

// "basepri" should be the value returned from raise_irq_pri
static inline void restore_irq_pri(uint32_t basepri) {
    __set_BASEPRI(basepri);
}

#endif // MICROPY_INCLUDED_AMBIQ_APOLLO_IRQ_H
