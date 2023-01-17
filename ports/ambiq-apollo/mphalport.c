#include "py/mphal.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "shared/timeutils/timeutils.h"
#include "uart.h"
#include "usbd.h"

#define MICROPY_HW_STDIN_BUFFER_LEN (256)

static uint8_t stdin_ringbuf_array[MICROPY_HW_STDIN_BUFFER_LEN];
ringbuf_t stdin_ringbuf = { stdin_ringbuf_array, sizeof(stdin_ringbuf_array), 0, 0 };

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {
    uintptr_t ret = 0;

    #if MICROPY_HW_USB_CDC
    usbd_cdc_poll();
    #endif

    if ((poll_flags & MP_STREAM_POLL_RD) && ringbuf_peek(&stdin_ringbuf) != -1) {
        ret |= MP_STREAM_POLL_RD;
    }

    #if MICROPY_PY_OS_DUPTERM
    ret |= mp_uos_dupterm_poll(poll_flags);
    #endif

    return ret;
}

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    uart_write(str, len);

    #if MICROPY_HW_USB_CDC
    usbd_cdc_write(str, len);
    #endif

    #if MICROPY_PY_OS_DUPTERM
    mp_uos_dupterm_tx_strn(str, len);
    #endif
}

int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        #if MICROPY_HW_USB_CDC
        usbd_cdc_poll();
        #endif

        //uart_process_incoming_char(0);
        int c = ringbuf_get(&stdin_ringbuf);
        if (c != -1) {
            return c;
        }

        #if MICROPY_PY_OS_DUPTERM
        int dupterm_c = mp_uos_dupterm_rx_chr();
        if (dupterm_c >= 0) {
            return dupterm_c;
        }
        #endif

        MICROPY_EVENT_POLL_HOOK;
    }
}

// The SysTick timer counts down at SYSCLK, so we can use that knowledge to grab a microsecond counter.
mp_uint_t mp_hal_ticks_us(void) {
    mp_uint_t irq_state = disable_irq();
    uint32_t counter = SysTick->VAL;
    uint32_t milliseconds = ticks_ms;
    uint32_t status = SysTick->CTRL;
    enable_irq(irq_state);

    // It's still possible for the countflag bit to get set if the counter was
    // reloaded between reading VAL and reading CTRL. With interrupts  disabled
    // it definitely takes less than 50 HCLK cycles between reading VAL and
    // reading CTRL, so the test (counter > 50) is to cover the case where VAL
    // is +ve and very close to zero, and the COUNTFLAG bit is also set.
    if ((status & SysTick_CTRL_COUNTFLAG_Msk) && counter > 50) {
        // This means that the HW reloaded VAL between the time we read VAL and the
        // time we read CTRL, which implies that there is an interrupt pending
        // to increment the tick counter.
        milliseconds++;
    }
    uint32_t load = SysTick->LOAD;
    counter = load - counter; // Convert from decrementing to incrementing

    // ((load + 1) / 1000) is the number of counts per microsecond.
    //
    // counter / ((load + 1) / 1000) scales from the systick clock to microseconds
    // and is the same thing as (counter * 1000) / (load + 1)
    return milliseconds * 1000 + (counter * 1000) / (load + 1);
}

void mp_hal_ticks_cpu_enable(void) {
    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

void mp_hal_delay_ms(mp_uint_t ms) {
    // TODO this assumes IRQs are enabled, so ticks_ms increases at 1Hz
    uint32_t start = ticks_ms;
    do {
        MICROPY_EVENT_POLL_HOOK;
    } while (ticks_ms - start < ms);
}
void am_util_delay_ms(uint32_t ms) {
    mp_hal_delay_ms(ms);
}

void mp_hal_delay_us(mp_uint_t us) {
    // TODO this assumes IRQs are enabled, so mp_hal_ticks_us works
    uint32_t start = mp_hal_ticks_us();
    while (mp_hal_ticks_us() - start < us) {
    }
}

uint64_t mp_hal_time_ns(void) {
    am_hal_rtc_time_t t;
    am_hal_rtc_time_get(&t);
    uint64_t s = timeutils_seconds_since_epoch(
        2000 + t.ui32Year, t.ui32Month, t.ui32DayOfMonth,
        t.ui32Hour, t.ui32Minute, t.ui32Second);
    s = s * 100ULL + t.ui32Hundredths;
    return s * 10000000ULL;
}
