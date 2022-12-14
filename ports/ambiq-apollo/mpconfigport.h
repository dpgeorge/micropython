#include <stdint.h>

#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES)

#define MICROPY_PERSISTENT_CODE_LOAD            (1)
#define MICROPY_EMIT_THUMB                      (1)
#define MICROPY_EMIT_INLINE_THUMB               (1)

#define MICROPY_READER_VFS                      (1)
#define MICROPY_ENABLE_GC                       (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF  (1)
#define MICROPY_LONGINT_IMPL                    (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL                      (MICROPY_FLOAT_IMPL_FLOAT)

#define MICROPY_EPOCH_IS_1970                   (1)
#define MICROPY_PY_MACHINE                      (1)
#define MICROPY_PY_MACHINE_SOFTI2C              (1)
#define MICROPY_PY_MACHINE_SOFTSPI              (1)
#define MICROPY_PY_UTIME_MP_HAL                 (1)
#define MICROPY_VFS                             (1)

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

#define MICROPY_HW_BOARD_NAME "AMAP4PEVB"
#define MICROPY_HW_MCU_NAME "AMAP4P"
#define MICROPY_EVENT_POLL_HOOK \
    mp_handle_pending(true); \

#define MP_STATE_PORT MP_STATE_VM

#include "lib/cmsis/inc/cmsis_gcc.h"

static inline void enable_irq(mp_uint_t state) {
    __set_PRIMASK(state);
}

static inline mp_uint_t disable_irq(void) {
    mp_uint_t state = __get_PRIMASK();
    __disable_irq();
    return state;
}

#define MICROPY_BEGIN_ATOMIC_SECTION()     disable_irq()
#define MICROPY_END_ATOMIC_SECTION(state)  enable_irq(state)
