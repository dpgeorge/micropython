/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022-2023 Damien P. George
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

#include "py/mphal.h"
#include "pin.h"

void mp_hal_pin_input(mp_hal_pin_obj_t pin) {
    am_hal_gpio_pincfg_t cfg = AMAP_PIN_PINCFG_INPUT;
    am_hal_gpio_pinconfig(pin, cfg);
}

void mp_hal_pin_output(mp_hal_pin_obj_t pin) {
    am_hal_gpio_pincfg_t cfg = AMAP_PIN_PINCFG_OUTPUT;
    am_hal_gpio_pinconfig(pin, cfg);
}

void mp_hal_pin_open_drain(mp_hal_pin_obj_t pin) {
    am_hal_gpio_pincfg_t cfg = AMAP_PIN_PINCFG_OPENDRAIN;
    am_hal_gpio_pinconfig(pin, cfg);
}
