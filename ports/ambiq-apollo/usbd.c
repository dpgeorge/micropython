#include <unistd.h>
#include "py/mphal.h"
#include "py/runtime.h"
#include "tusb.h"
#include "usbd.h"

#if MICROPY_HW_USB_CDC

#define MICROPY_HW_USB_CDC_TX_TIMEOUT (100) // in ms

uint8_t cdc_itf_pending; // keep track of cdc interfaces which need attention to poll

extern void *pUSBHandle;

void USB_IRQHandler(void) {
    //printf("[%08d] USB_IRQHandler enter\n", usbd_ticks());
    uint32_t status[3];
    am_hal_usb_intr_status_get(pUSBHandle, &status[0], &status[1], &status[2]);
    am_hal_usb_interrupt_service(pUSBHandle, status[0], status[1], status[2]);
    //printf("[%08d] USB_IRQHandler exit\n", usbd_ticks());
}

void usbd_cdc_poll(void) {
    // any CDC interfaces left to poll?
    if (cdc_itf_pending && ringbuf_free(&stdin_ringbuf)) {
        for (uint8_t itf = 0; itf < 8; ++itf) {
            if (cdc_itf_pending & (1 << itf)) {
                tud_cdc_rx_cb(itf);
                if (!cdc_itf_pending) {
                    break;
                }
            }
        }
    }
}

void tud_cdc_rx_cb(uint8_t itf) {
    // consume pending USB data immediately to free usb buffer and keep the endpoint from stalling.
    // in case the ringbuffer is full, mark the CDC interface that need attention later on for polling
    cdc_itf_pending &= ~(1 << itf);
    for (uint32_t bytes_avail = tud_cdc_n_available(itf); bytes_avail > 0; --bytes_avail) {
        if (ringbuf_free(&stdin_ringbuf)) {
            int data_char = tud_cdc_read_char();
            if (data_char == mp_interrupt_char) {
                mp_sched_keyboard_interrupt();
            } else {
                ringbuf_put(&stdin_ringbuf, data_char);
            }
        } else {
            cdc_itf_pending |= (1 << itf);
            return;
        }
    }
}

void usbd_cdc_write(const char *str, size_t len) {
    if (tud_cdc_connected()) {
        for (size_t i = 0; i < len;) {
            uint32_t n = len - i;
            if (n > 32 /*CFG_TUD_CDC_EP_BUFSIZE*/) {
                n = 32 /*CFG_TUD_CDC_EP_BUFSIZE*/;
            }
            // Wait with a max of USC_CDC_TIMEOUT ms
            mp_uint_t t0 = mp_hal_ticks_ms();
            while (n > tud_cdc_write_available()) {
                if ((uint32_t)(mp_hal_ticks_ms() - t0) > MICROPY_HW_USB_CDC_TX_TIMEOUT) {
                    return;
                }
                MICROPY_EVENT_POLL_HOOK;
                __WFI();
            }
            uint32_t n2 = tud_cdc_write(str + i, n);
            tud_cdc_write_flush();
            MICROPY_EVENT_POLL_HOOK;
            i += n2;
        }
    }
}

#endif
