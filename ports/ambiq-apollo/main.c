#include "py/gc.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/stackctrl.h"
#include "shared/readline/readline.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "tusb.h"
#include "modmachine.h"

extern uint32_t _sstack, _estack, _sidata, _sdata, _edata, _sbss, _ebss;

static char gc_heap[256 * 1024];

void amap_main(void) {
    tusb_init();

    // Initialise stack extents and GC heap.
    mp_stack_set_top(&_estack);
    mp_stack_set_limit((char *)&_estack - (char *)&_sstack - 512);
    gc_init(&gc_heap[0], &gc_heap[MP_ARRAY_SIZE(gc_heap)]);

    #if 0
    char buf[4];
    int count = 0;
    #endif
    for (;;) {
        // Initialise MicroPython runtime.
        mp_init();
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));

        // Initialise sub-systems.
        readline_init0();
        machine_pin_init();

        // Execute user scripts.
        int ret = pyexec_file_if_exists("boot.py");
        if (ret & PYEXEC_FORCED_EXIT) {
            goto soft_reset_exit;
        }
        if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
            ret = pyexec_file_if_exists("main.py");
            if (ret & PYEXEC_FORCED_EXIT) {
                goto soft_reset_exit;
            }
        }

        for (;;) {
            #if 0
            ++count;
            buf[0] = '0' + ((count / 10) % 10);
            buf[1] = '0' + (count % 10);
            buf[2] = '\r';
            buf[3] = '\n';
    usbd_cdc_write(buf, 4);
    usbd_cdc_write(
        "0a.........b.........c.........d.........e.........f.........g.........h.........i.........j.........k\r\n"
        "1a.........b.........c.........d.........e.........f.........g.........h.........i.........j.........k\r\n"
        "2a.........b.........c.........d.........e.........f.........g.........h.........i.........j.........k\r\n"
        "3a.........b.........c.........d.........e.........f.........g.........h.........i.........j.........k\r\n"
        "4a.........b.........c.........d.........e.........f.........g.........h.........i.........j.........k\r\n"
        "5a.........b.........c.........d.........e.........f.........g.........h.........i.........j.........k\r\n"
        , 6*104);
    #endif

            if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                if (pyexec_raw_repl() != 0) {
                    break;
                }
            } else {
                if (pyexec_friendly_repl() != 0) {
                    break;
                }
            }
        }

    soft_reset_exit:
        mp_printf(MP_PYTHON_PRINTER, "MPY: soft reboot\n");
        machine_pin_deinit();
        gc_sweep_all();
        mp_deinit();
    }
}

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    gc_collect_end();
}

void nlr_jump_fail(void *val) {
    while (1) {
        ;
    }
}

void NORETURN __fatal_error(const char *msg) {
    while (1) {
        ;
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif
