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
#ifndef MICROPY_INCLUDED_AMBIQ_PIN_H
#define MICROPY_INCLUDED_AMBIQ_PIN_H

#define AMAP_PIN_PINCFG_INPUT                                               \
    {                                                                       \
        .GP.cfg_b.uFuncSel          = 3,                                    \
        .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,       \
        .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,   \
        .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_NONE,          \
        .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_ENABLE,         \
        .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,       \
        .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,         \
        .GP.cfg_b.uSlewRate         = 0,                                    \
        .GP.cfg_b.uNCE              = 0,                                    \
        .GP.cfg_b.eCEpol            = 0,                                    \
        .GP.cfg_b.ePowerSw          = 0,                                    \
        .GP.cfg_b.eForceInputEn     = 0,                                    \
        .GP.cfg_b.eForceOutputEn    = 0,                                    \
        .GP.cfg_b.uRsvd_0           = 0,                                    \
        .GP.cfg_b.uRsvd_1           = 0,                                    \
    }

#define AMAP_PIN_PINCFG_OUTPUT                                              \
    {                                                                       \
        .GP.cfg_b.uFuncSel          = 3,                                    \
        .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,      \
        .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,   \
        .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_NONE,          \
        .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_ENABLE,         \
        .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,       \
        .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,         \
        .GP.cfg_b.uSlewRate         = 0,                                    \
        .GP.cfg_b.uNCE              = 0,                                    \
        .GP.cfg_b.eCEpol            = 0,                                    \
        .GP.cfg_b.ePowerSw          = 0,                                    \
        .GP.cfg_b.eForceInputEn     = 0,                                    \
        .GP.cfg_b.eForceOutputEn    = 0,                                    \
        .GP.cfg_b.uRsvd_0           = 0,                                    \
        .GP.cfg_b.uRsvd_1           = 0,                                    \
    }

#define AMAP_PIN_PINCFG_OPENDRAIN                                           \
    {                                                                       \
        .GP.cfg_b.uFuncSel          = 3,                                    \
        .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,     \
        .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,   \
        .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_NONE,          \
        .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_ENABLE,         \
        .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,       \
        .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,         \
        .GP.cfg_b.uSlewRate         = 0,                                    \
        .GP.cfg_b.uNCE              = 0,                                    \
        .GP.cfg_b.eCEpol            = 0,                                    \
        .GP.cfg_b.ePowerSw          = 0,                                    \
        .GP.cfg_b.eForceInputEn     = 0,                                    \
        .GP.cfg_b.eForceOutputEn    = 0,                                    \
        .GP.cfg_b.uRsvd_0           = 0,                                    \
        .GP.cfg_b.uRsvd_1           = 0,                                    \
    }

#define AMAP_PIN_PINCFG_TRISTATE                                            \
    {                                                                       \
        .GP.cfg_b.uFuncSel          = 3,                                    \
        .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_TRISTATE,      \
        .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,   \
        .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_NONE,          \
        .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_ENABLE,         \
        .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,       \
        .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,         \
        .GP.cfg_b.uSlewRate         = 0,                                    \
        .GP.cfg_b.uNCE              = 0,                                    \
        .GP.cfg_b.eCEpol            = 0,                                    \
        .GP.cfg_b.ePowerSw          = 0,                                    \
        .GP.cfg_b.eForceInputEn     = 0,                                    \
        .GP.cfg_b.eForceOutputEn    = 0,                                    \
        .GP.cfg_b.uRsvd_0           = 0,                                    \
        .GP.cfg_b.uRsvd_1           = 0,                                    \
    }

#endif // MICROPY_INCLUDED_AMBIQ_PIN_H
