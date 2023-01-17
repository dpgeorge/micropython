#include "py/mphal.h"
#include "py/runtime.h"
#include "am_mcu_apollo.h"
#include "uart.h"

static inline void uart_write_char(int c) {
    uint32_t uart_id = 0;
    while (UARTn(uart_id)->FR_b.TXFF) {
    }
    UARTn(uart_id)->DR = c;
}

static void uart_process_incoming_char(uint32_t uart_id) {
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

void uart_write(const char *str, size_t len) {
    while (len--) {
        uart_write_char(*str++);
    }
}
