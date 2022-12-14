#include <unistd.h>
#include "py/mphal.h"
#include "py/ringbuf.h"
#include "py/stream.h"
#include "extmod/misc.h"
#include "am_mcu_apollo.h"

#define MICROPY_HW_STDIN_BUFFER_LEN (256)

static uint8_t stdin_ringbuf_array[MICROPY_HW_STDIN_BUFFER_LEN];
static ringbuf_t stdin_ringbuf = { stdin_ringbuf_array, sizeof(stdin_ringbuf_array), 0, 0 };

static inline void uart_write_char(int c) {
    uint32_t uart_id = 0;
    while (UARTn(uart_id)->FR_b.TXFF) {
    }
    UARTn(uart_id)->DR = c;
}

void uart_process_incoming_char(uint32_t uart_id) {
    while (!UARTn(uart_id)->FR_b.RXFE && ringbuf_free(&stdin_ringbuf)) {
        int c = UARTn(uart_id)->DR;
        if (c == mp_interrupt_char) {
            stdin_ringbuf.iget = stdin_ringbuf.iput = 0;
            mp_sched_keyboard_interrupt();
        } else {
            ringbuf_put(&stdin_ringbuf, c);
        }
    }
}

extern void *phUART;
void UART0_IRQHandler(void) {
    /*
    uint32_t ui32Status;
    am_hal_uart_interrupt_status_get(phUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(phUART, ui32Status);
    am_hal_uart_interrupt_service(phUART, ui32Status);
    */
    uint32_t uart_id = 0;
    if (UARTn(uart_id)->IES & (UART0_IER_RTIM_Msk | UART0_IER_RXIM_Msk)) {
        UARTn(uart_id)->IEC = UART0_IEC_RTIC_Msk | UART0_IEC_RXIC_Msk;
        uart_process_incoming_char(uart_id);
    }
}

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {
    uintptr_t ret = 0;

    if ((poll_flags & MP_STREAM_POLL_RD) && ringbuf_peek(&stdin_ringbuf) != -1) {
        ret |= MP_STREAM_POLL_RD;
    }

    #if MICROPY_PY_OS_DUPTERM
    ret |= mp_uos_dupterm_poll(poll_flags);
    #endif

    return ret;
}

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    while (len--) {
        uart_write_char(*str++);
    }
}

int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        //uart_process_incoming_char(0);
        int c = ringbuf_get(&stdin_ringbuf);
        if (c != -1) {
            return c;
        }
        MICROPY_EVENT_POLL_HOOK;
    }
}
