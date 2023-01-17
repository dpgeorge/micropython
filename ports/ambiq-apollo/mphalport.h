#include "py/obj.h"
#include "py/ringbuf.h"
#include "shared/runtime/interrupt_char.h"
#include "am_mcu_apollo.h"

extern uint32_t ticks_ms;

//extern int mp_interrupt_char;
extern ringbuf_t stdin_ringbuf;

void mp_hal_ticks_cpu_enable(void);
void mp_hal_delay_us(mp_uint_t us);

static inline mp_uint_t mp_hal_ticks_ms(void) {
    return ticks_ms;
}

static inline mp_uint_t mp_hal_ticks_cpu(void) {
    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk)) {
        mp_hal_ticks_cpu_enable();
    }
    return DWT->CYCCNT;
}

static inline void mp_hal_delay_us_fast(uint32_t us) {
    mp_hal_delay_us(us);
}

// C-level pin HAL

#define MP_HAL_PIN_FMT "%u"
#define mp_hal_pin_obj_t unsigned int

mp_hal_pin_obj_t mp_hal_get_pin_obj(mp_obj_t pin);

static inline unsigned int mp_hal_pin_name(mp_hal_pin_obj_t pin) {
    return pin;
}

void mp_hal_pin_input(mp_hal_pin_obj_t pin);
void mp_hal_pin_output(mp_hal_pin_obj_t pin);
void mp_hal_pin_open_drain(mp_hal_pin_obj_t pin);

static inline int mp_hal_pin_read(mp_hal_pin_obj_t pin) {
    return am_hal_gpio_input_read(pin);
}

static inline void mp_hal_pin_write(mp_hal_pin_obj_t pin, int v) {
    if (v) {
        am_hal_gpio_output_set(pin);
    } else {
        am_hal_gpio_output_clear(pin);
    }
}

static inline void mp_hal_pin_od_low(mp_hal_pin_obj_t pin) {
    am_hal_gpio_output_clear(pin);
}

static inline void mp_hal_pin_od_high(mp_hal_pin_obj_t pin) {
    am_hal_gpio_output_set(pin);
}
