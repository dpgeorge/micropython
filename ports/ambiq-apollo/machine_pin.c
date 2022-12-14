/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2022 Damien P. George
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

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "shared/runtime/mpirq.h"
#include "modmachine.h"
#include "extmod/virtpin.h"
#include "am_mcu_apollo.h"

#define GPIO_MODE_IN (0)
#define GPIO_MODE_OUT (1)
#define GPIO_MODE_OPEN_DRAIN (2)
#define GPIO_MODE_TRISTATE (3)
#define GPIO_MODE_ALT (4)

#define GPIO_IRQ_ALL (0xf)

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint8_t id;
} machine_pin_obj_t;

#if 0
typedef struct _machine_pin_irq_obj_t {
    mp_irq_obj_t base;
    uint32_t flags;
    uint32_t trigger;
} machine_pin_irq_obj_t;

STATIC const mp_irq_methods_t machine_pin_irq_methods;
#endif

STATIC const machine_pin_obj_t machine_pin_obj[AM_HAL_GPIO_MAX_PADS] = {
    {{&machine_pin_type}, 0},
    {{&machine_pin_type}, 1},
    {{&machine_pin_type}, 2},
    {{&machine_pin_type}, 3},
    {{&machine_pin_type}, 4},
    {{&machine_pin_type}, 5},
    {{&machine_pin_type}, 6},
    {{&machine_pin_type}, 7},
    {{&machine_pin_type}, 8},
    {{&machine_pin_type}, 9},
    {{&machine_pin_type}, 10},
    {{&machine_pin_type}, 11},
    {{&machine_pin_type}, 12},
    {{&machine_pin_type}, 13},
    {{&machine_pin_type}, 14},
    {{&machine_pin_type}, 15},
    {{&machine_pin_type}, 16},
    {{&machine_pin_type}, 17},
    {{&machine_pin_type}, 18},
    {{&machine_pin_type}, 19},
    {{&machine_pin_type}, 20},
    {{&machine_pin_type}, 21},
    {{&machine_pin_type}, 22},
    {{&machine_pin_type}, 23},
    {{&machine_pin_type}, 24},
    {{&machine_pin_type}, 25},
    {{&machine_pin_type}, 26},
    {{&machine_pin_type}, 27},
    {{&machine_pin_type}, 28},
    {{&machine_pin_type}, 29},
    {{&machine_pin_type}, 30},
    {{&machine_pin_type}, 31},
    {{&machine_pin_type}, 32},
    {{&machine_pin_type}, 33},
    {{&machine_pin_type}, 34},
    {{&machine_pin_type}, 35},
    {{&machine_pin_type}, 36},
    {{&machine_pin_type}, 37},
    {{&machine_pin_type}, 38},
    {{&machine_pin_type}, 39},
    {{&machine_pin_type}, 40},
    {{&machine_pin_type}, 41},
    {{&machine_pin_type}, 42},
    {{&machine_pin_type}, 43},
    {{&machine_pin_type}, 44},
    {{&machine_pin_type}, 45},
    {{&machine_pin_type}, 46},
    {{&machine_pin_type}, 47},
    {{&machine_pin_type}, 48},
    {{&machine_pin_type}, 49},
    {{&machine_pin_type}, 50},
    {{&machine_pin_type}, 51},
    {{&machine_pin_type}, 52},
    {{&machine_pin_type}, 53},
    {{&machine_pin_type}, 54},
    {{&machine_pin_type}, 55},
    {{&machine_pin_type}, 56},
    {{&machine_pin_type}, 57},
    {{&machine_pin_type}, 58},
    {{&machine_pin_type}, 59},
    {{&machine_pin_type}, 60},
    {{&machine_pin_type}, 61},
    {{&machine_pin_type}, 62},
    {{&machine_pin_type}, 63},
    {{&machine_pin_type}, 64},
    {{&machine_pin_type}, 65},
    {{&machine_pin_type}, 66},
    {{&machine_pin_type}, 67},
    {{&machine_pin_type}, 68},
    {{&machine_pin_type}, 69},
    {{&machine_pin_type}, 70},
    {{&machine_pin_type}, 71},
    {{&machine_pin_type}, 72},
    {{&machine_pin_type}, 73},
    {{&machine_pin_type}, 74},
    {{&machine_pin_type}, 75},
    {{&machine_pin_type}, 76},
    {{&machine_pin_type}, 77},
    {{&machine_pin_type}, 78},
    {{&machine_pin_type}, 79},
    {{&machine_pin_type}, 80},
    {{&machine_pin_type}, 81},
    {{&machine_pin_type}, 82},
    {{&machine_pin_type}, 83},
    {{&machine_pin_type}, 84},
    {{&machine_pin_type}, 85},
    {{&machine_pin_type}, 86},
    {{&machine_pin_type}, 87},
    {{&machine_pin_type}, 88},
    {{&machine_pin_type}, 89},
    {{&machine_pin_type}, 90},
    {{&machine_pin_type}, 91},
    {{&machine_pin_type}, 92},
    {{&machine_pin_type}, 93},
    {{&machine_pin_type}, 94},
    {{&machine_pin_type}, 95},
    {{&machine_pin_type}, 96},
    {{&machine_pin_type}, 97},
    {{&machine_pin_type}, 98},
    {{&machine_pin_type}, 99},
    {{&machine_pin_type}, 100},
    {{&machine_pin_type}, 101},
    {{&machine_pin_type}, 102},
    {{&machine_pin_type}, 103},
    {{&machine_pin_type}, 104},
    {{&machine_pin_type}, 105},
    {{&machine_pin_type}, 106},
    {{&machine_pin_type}, 107},
    {{&machine_pin_type}, 108},
    {{&machine_pin_type}, 109},
    {{&machine_pin_type}, 110},
    {{&machine_pin_type}, 111},
    {{&machine_pin_type}, 112},
    {{&machine_pin_type}, 113},
    {{&machine_pin_type}, 114},
    {{&machine_pin_type}, 115},
    {{&machine_pin_type}, 116},
    {{&machine_pin_type}, 117},
    {{&machine_pin_type}, 118},
    {{&machine_pin_type}, 119},
    {{&machine_pin_type}, 120},
    {{&machine_pin_type}, 121},
    {{&machine_pin_type}, 122},
    {{&machine_pin_type}, 123},
    {{&machine_pin_type}, 124},
    {{&machine_pin_type}, 125},
    {{&machine_pin_type}, 126},
    {{&machine_pin_type}, 127},
};

void machine_pin_init(void) {
}

void machine_pin_deinit(void) {
}

STATIC const qstr_short_t pin_mode_qstr_table[] = {
    MP_QSTR_IN,
    MP_QSTR_OUT,
    MP_QSTR_OPEN_DRAIN,
    MP_QSTR_TRISTATE,
};

STATIC const qstr_short_t pin_pull_qstr_table[] = {
    MP_QSTR_,
    MP_QSTR_PULL_DOWN_50K,
    MP_QSTR_PULL_UP_1_5K,
    MP_QSTR_PULL_UP_6K,
    MP_QSTR_PULL_UP_12K,
    MP_QSTR_PULL_UP_24K,
    MP_QSTR_PULL_UP_50K,
    MP_QSTR_PULL_UP_100K,
};

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    am_hal_gpio_pincfg_t cfg;
    am_hal_gpio_pinconfig_get(self->id, &cfg);
    mp_printf(print, "Pin(%u", self->id);
    qstr mode_qst = MP_QSTRnull;
    if (cfg.GP.cfg_b.uFuncSel == 3) {
        if (cfg.GP.cfg_b.eGPOutCfg == AM_HAL_GPIO_PIN_OUTCFG_DISABLE) {
            if (cfg.GP.cfg_b.eGPInput == AM_HAL_GPIO_PIN_INPUT_ENABLE) {
                mode_qst = MP_QSTR_IN;
            }
        } else {
            mode_qst = pin_mode_qstr_table[cfg.GP.cfg_b.eGPOutCfg];
        }
    } else {
        mode_qst = MP_QSTR_ALT;
    }
    if (mode_qst != MP_QSTRnull) {
        mp_printf(print, ", mode=%q", mode_qst);
    }
    if (cfg.GP.cfg_b.ePullup != AM_HAL_GPIO_PIN_PULLUP_NONE) {
        mp_printf(print, ", pull=%q", pin_pull_qstr_table[cfg.GP.cfg_b.ePullup]);
    }
    if (cfg.GP.cfg_b.uFuncSel != 3) {
        mp_printf(print, ", alt=%u", cfg.GP.cfg_b.uFuncSel);
    }
    mp_printf(print, ")");
}

enum {
    ARG_mode, ARG_pull, ARG_value, ARG_alt
};
static const mp_arg_t allowed_args[] = {
    {MP_QSTR_mode,  MP_ARG_OBJ,                  {.u_rom_obj = MP_ROM_NONE}},
    {MP_QSTR_pull,  MP_ARG_OBJ,                  {.u_rom_obj = MP_ROM_NONE}},
    {MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE}},
    {MP_QSTR_alt,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 3}},
};

STATIC mp_obj_t machine_pin_obj_init_helper(const machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Set initial value (do this before configuring mode/pull).
    if (args[ARG_value].u_obj != mp_const_none) {
        if (mp_obj_is_true(args[ARG_value].u_obj)) {
            am_hal_gpio_output_set(self->id);
        } else {
            am_hal_gpio_output_clear(self->id);
        }
    }

    // Get current pin configuration.
    am_hal_gpio_pincfg_t cfg;
    am_hal_gpio_pinconfig_get(self->id, &cfg);

    // Configure mode.
    if (args[ARG_mode].u_obj != mp_const_none) {
        mp_int_t mode = mp_obj_get_int(args[ARG_mode].u_obj);
        if (mode == GPIO_MODE_IN) {
            cfg = (am_hal_gpio_pincfg_t)AM_HAL_GPIO_PINCFG_INPUT;
        } else if (mode == GPIO_MODE_OUT) {
            cfg = (am_hal_gpio_pincfg_t)AM_HAL_GPIO_PINCFG_OUTPUT_WITH_READ;
        } else if (mode == GPIO_MODE_OPEN_DRAIN) {
            cfg = (am_hal_gpio_pincfg_t)AM_HAL_GPIO_PINCFG_OPENDRAIN;
        } else if (mode == GPIO_MODE_TRISTATE) {
            cfg = (am_hal_gpio_pincfg_t)AM_HAL_GPIO_PINCFG_TRISTATE;
        } else {
            // Alternate function.
            // TODO
            //gpio_set_function(self->id, args[ARG_alt].u_int);
        }
    }

    // Configure pull (unconditionally because None means no-pull).
    uint32_t pull = AM_HAL_GPIO_PIN_PULLUP_NONE;
    if (args[ARG_pull].u_obj != mp_const_none) {
        pull = mp_obj_get_int(args[ARG_pull].u_obj);
    }
    cfg.GP.cfg_b.ePullup = pull;

    // Set new pin configuration.
    am_hal_gpio_pinconfig(self->id, cfg);

    return mp_const_none;
}

// constructor(id, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // Get the wanted pin object.
    int wanted_pin = mp_obj_get_int(args[0]);
    if (!(0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj))) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid pin"));
    }
    const machine_pin_obj_t *self = &machine_pin_obj[wanted_pin];

    if (n_args > 1 || n_kw > 0) {
        // Pin mode given, so configure this GPIO.
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    if (n_args == 0) {
        // Get pin.
        return MP_OBJ_NEW_SMALL_INT(am_hal_gpio_input_read(self->id));
    } else {
        // Set pin.
        bool value = mp_obj_is_true(args[0]);
        if (value) {
            am_hal_gpio_output_set(self->id);
        } else {
            am_hal_gpio_output_clear(self->id);
        }
        return mp_const_none;
    }
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// pin.low()
STATIC mp_obj_t machine_pin_low(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    am_hal_gpio_output_clear(self->id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_low_obj, machine_pin_low);

// pin.high()
STATIC mp_obj_t machine_pin_high(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    am_hal_gpio_output_set(self->id);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_high_obj, machine_pin_high);

// pin.toggle()
STATIC mp_obj_t machine_pin_toggle(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    am_hal_gpio_output_toggle(self->id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_toggle_obj, machine_pin_toggle);

#if 0
STATIC machine_pin_irq_obj_t *machine_pin_get_irq(mp_hal_pin_obj_t pin) {
    // Get the IRQ object.
    machine_pin_irq_obj_t *irq = MP_STATE_PORT(machine_pin_irq_obj[pin]);

    // Allocate the IRQ object if it doesn't already exist.
    if (irq == NULL) {
        irq = m_new_obj(machine_pin_irq_obj_t);
        irq->base.base.type = &mp_irq_type;
        irq->base.methods = (mp_irq_methods_t *)&machine_pin_irq_methods;
        irq->base.parent = MP_OBJ_FROM_PTR(&machine_pin_obj[pin]);
        irq->base.handler = mp_const_none;
        irq->base.ishard = false;
        MP_STATE_PORT(machine_pin_irq_obj[pin]) = irq;
    }
    return irq;
}

void mp_hal_pin_interrupt(mp_hal_pin_obj_t pin, mp_obj_t handler, mp_uint_t trigger, bool hard) {
    machine_pin_irq_obj_t *irq = machine_pin_get_irq(pin);

    // Disable all IRQs while data is updated.
    gpio_set_irq_enabled(pin, GPIO_IRQ_ALL, false);

    // Update IRQ data.
    irq->base.handler = handler;
    irq->base.ishard = hard;
    irq->flags = 0;
    irq->trigger = trigger;

    // Enable IRQ if a handler is given.
    if (handler != mp_const_none && trigger != MP_HAL_PIN_TRIGGER_NONE) {
        gpio_set_irq_enabled(pin, trigger, true);
    }
}

// pin.irq(handler=None, trigger=IRQ_FALLING|IRQ_RISING, hard=False)
STATIC mp_obj_t machine_pin_irq(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_handler, ARG_trigger, ARG_hard };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_handler, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_trigger, MP_ARG_INT, {.u_int = MP_HAL_PIN_TRIGGER_FALL | MP_HAL_PIN_TRIGGER_RISE} },
        { MP_QSTR_hard, MP_ARG_BOOL, {.u_bool = false} },
    };
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    if (is_cyw43_pin(self)) {
        mp_raise_ValueError(MP_ERROR_TEXT("expecting a regular GPIO Pin"));
    }

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    machine_pin_irq_obj_t *irq = machine_pin_get_irq(self->id);

    if (n_args > 1 || kw_args->used != 0) {
        // Update IRQ data.
        mp_obj_t handler = args[ARG_handler].u_obj;
        mp_uint_t trigger = args[ARG_trigger].u_int;
        bool hard = args[ARG_hard].u_bool;
        mp_hal_pin_interrupt(self->id, handler, trigger, hard);
    }
    return MP_OBJ_FROM_PTR(irq);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_irq_obj, 1, machine_pin_irq);
#endif

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_low), MP_ROM_PTR(&machine_pin_low_obj) },
    { MP_ROM_QSTR(MP_QSTR_high), MP_ROM_PTR(&machine_pin_high_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&machine_pin_low_obj) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&machine_pin_high_obj) },
    { MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&machine_pin_toggle_obj) },
    //{ MP_ROM_QSTR(MP_QSTR_irq), MP_ROM_PTR(&machine_pin_irq_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_MODE_IN) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_MODE_OUT) },
    { MP_ROM_QSTR(MP_QSTR_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_OPEN_DRAIN) },
    { MP_ROM_QSTR(MP_QSTR_TRISTATE), MP_ROM_INT(GPIO_MODE_TRISTATE) },
    { MP_ROM_QSTR(MP_QSTR_ALT), MP_ROM_INT(GPIO_MODE_ALT) },

    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLUP_50K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLDOWN_50K) },

    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN_50K), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLDOWN_50K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_1_5K), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLUP_1_5K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_6K), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLUP_1_5K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_12K), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLUP_12K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_24K), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLUP_24K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_50K), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLUP_50K) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP_100K), MP_ROM_INT(AM_HAL_GPIO_PIN_PULLUP_100K) },

    //{ MP_ROM_QSTR(MP_QSTR_IRQ_RISING), MP_ROM_INT(GPIO_IRQ_EDGE_RISE) },
    //{ MP_ROM_QSTR(MP_QSTR_IRQ_FALLING), MP_ROM_INT(GPIO_IRQ_EDGE_FALL) },
};
STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

STATIC mp_uint_t pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    (void)errcode;
    machine_pin_obj_t *self = self_in;
    switch (request) {
        case MP_PIN_READ: {
            return am_hal_gpio_input_read(self->id);
        }
        case MP_PIN_WRITE: {
            if (arg) {
                am_hal_gpio_output_set(self->id);
            } else {
                am_hal_gpio_output_clear(self->id);
            }
            return 0;
        }
    }
    return -1;
}

STATIC const mp_pin_p_t pin_pin_p = {
    .ioctl = pin_ioctl,
};

MP_DEFINE_CONST_OBJ_TYPE(
    machine_pin_type,
    MP_QSTR_Pin,
    MP_TYPE_FLAG_NONE,
    make_new, mp_pin_make_new,
    print, machine_pin_print,
    call, machine_pin_call,
    protocol, &pin_pin_p,
    locals_dict, &machine_pin_locals_dict
    );

#if 0
STATIC mp_uint_t machine_pin_irq_trigger(mp_obj_t self_in, mp_uint_t new_trigger) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    machine_pin_irq_obj_t *irq = MP_STATE_PORT(machine_pin_irq_obj[self->id]);
    gpio_set_irq_enabled(self->id, GPIO_IRQ_ALL, false);
    irq->flags = 0;
    irq->trigger = new_trigger;
    gpio_set_irq_enabled(self->id, new_trigger, true);
    return 0;
}

STATIC mp_uint_t machine_pin_irq_info(mp_obj_t self_in, mp_uint_t info_type) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    machine_pin_irq_obj_t *irq = MP_STATE_PORT(machine_pin_irq_obj[self->id]);
    if (info_type == MP_IRQ_INFO_FLAGS) {
        return irq->flags;
    } else if (info_type == MP_IRQ_INFO_TRIGGERS) {
        return irq->trigger;
    }
    return 0;
}

STATIC const mp_irq_methods_t machine_pin_irq_methods = {
    .trigger = machine_pin_irq_trigger,
    .info = machine_pin_irq_info,
};

MP_REGISTER_ROOT_POINTER(void *machine_pin_irq_obj[30]);
#endif

///////////////////////////
// Pin HAL

mp_hal_pin_obj_t mp_hal_get_pin_obj(mp_obj_t pin_in) {
    if (!mp_obj_is_type(pin_in, &machine_pin_type)) {
        mp_raise_ValueError(MP_ERROR_TEXT("expecting a Pin"));
    }
    machine_pin_obj_t *pin = MP_OBJ_TO_PTR(pin_in);
    return pin->id;
}

void mp_hal_pin_input(mp_hal_pin_obj_t pin) {
    am_hal_gpio_pincfg_t cfg;
    cfg = (am_hal_gpio_pincfg_t)AM_HAL_GPIO_PINCFG_INPUT;
    am_hal_gpio_pinconfig(pin, cfg);
}

void mp_hal_pin_output(mp_hal_pin_obj_t pin) {
    am_hal_gpio_pincfg_t cfg;
    cfg = (am_hal_gpio_pincfg_t)AM_HAL_GPIO_PINCFG_OUTPUT;
    am_hal_gpio_pinconfig(pin, cfg);
}

void mp_hal_pin_open_drain(mp_hal_pin_obj_t pin) {
    am_hal_gpio_pincfg_t cfg;
    cfg = (am_hal_gpio_pincfg_t)AM_HAL_GPIO_PINCFG_OPENDRAIN;
    am_hal_gpio_pinconfig(pin, cfg);
}
