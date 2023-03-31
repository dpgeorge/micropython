#include <stdbool.h>
#include <stdint.h>
#include "irq.h"

#define MICROPY_HW_BOARD_NAME                   "AMAP4PEVB"
#define MICROPY_HW_MCU_NAME                     "AMAP4P"
#define MICROPY_HW_ENABLE_USBDEV                (1)
#define MICROPY_HW_USB_CDC                      (1)

#define MICROPY_CONFIG_ROM_LEVEL                (MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES)

#define MICROPY_PERSISTENT_CODE_LOAD            (1)
#define MICROPY_EMIT_THUMB                      (1)
#define MICROPY_EMIT_INLINE_THUMB               (1)

#define MICROPY_READER_VFS                      (1)
#define MICROPY_ENABLE_GC                       (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF  (1)
#define MICROPY_LONGINT_IMPL                    (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL                      (MICROPY_FLOAT_IMPL_FLOAT)

#define MICROPY_EPOCH_IS_1970                   (1)
#define MICROPY_PY_SYS_PLATFORM                 "amap"
#define MICROPY_PY_MACHINE                      (1)
#define MICROPY_PY_MACHINE_SOFTI2C              (1)
#define MICROPY_PY_MACHINE_SOFTSPI              (1)
#define MICROPY_PY_UTIME_MP_HAL                 (1)
#define MICROPY_PY_NETWORK                      (1)
#define MICROPY_VFS                             (1)

// fatfs configuration used in ffconf.h
#define MICROPY_FATFS_ENABLE_LFN                (1)
#define MICROPY_FATFS_LFN_CODE_PAGE             437 /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_FATFS_USE_LABEL                 (1)
#define MICROPY_FATFS_RPATH                     (2)
#define MICROPY_FATFS_MULTI_PARTITION           (1)

// Type definitions for the specific machine.

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void *)((uint32_t)(p) | 1))

#define MP_SSIZE_MAX (0x7fffffff)
typedef intptr_t mp_int_t; // must be pointer size
typedef uintptr_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// Need an implementation of the log2 function which is not a macro.
#define MP_NEED_LOG2 (1)

// Need to provide a declaration/definition of alloca().
#include <alloca.h>

#define MICROPY_EVENT_POLL_HOOK \
    usbd_task(); \
    mp_handle_pending(true); \

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_BEGIN_ATOMIC_SECTION()     disable_irq()
#define MICROPY_END_ATOMIC_SECTION(state)  enable_irq(state)

// For regular code that wants to prevent "background tasks" from running.
// These background tasks (LWIP, Bluetooth) run in PENDSV context.
#define MICROPY_PY_PENDSV_ENTER   uint32_t atomic_state = raise_irq_pri(IRQ_PRI_PENDSV);
#define MICROPY_PY_PENDSV_REENTER atomic_state = raise_irq_pri(IRQ_PRI_PENDSV);
#define MICROPY_PY_PENDSV_EXIT    restore_irq_pri(atomic_state);

// Prevent the "LWIP task" from running.
#define MICROPY_PY_LWIP_ENTER   MICROPY_PY_PENDSV_ENTER
#define MICROPY_PY_LWIP_REENTER MICROPY_PY_PENDSV_REENTER
#define MICROPY_PY_LWIP_EXIT    MICROPY_PY_PENDSV_EXIT

#ifndef MICROPY_PY_NETWORK_HOSTNAME_DEFAULT
#define MICROPY_PY_NETWORK_HOSTNAME_DEFAULT "mpy-amap"
#endif

#if MICROPY_PY_NETWORK_CYW43
extern const struct _mp_obj_type_t mp_network_cyw43_type;
#define MICROPY_HW_NIC_CYW43                { MP_ROM_QSTR(MP_QSTR_WLAN), MP_ROM_PTR(&mp_network_cyw43_type) },
#else
#define MICROPY_HW_NIC_CYW43
#endif

#ifndef MICROPY_BOARD_NETWORK_INTERFACES
#define MICROPY_BOARD_NETWORK_INTERFACES
#endif

#define MICROPY_PORT_NETWORK_INTERFACES \
    MICROPY_HW_NIC_CYW43 \
    MICROPY_BOARD_NETWORK_INTERFACES \

void mp_handle_pending(bool raise_exc);
void usbd_task(void);
