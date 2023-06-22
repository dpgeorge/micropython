/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Damien P. George
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

#include "emscripten.h"
#include "py/objmodule.h"
#include "py/runtime.h"
#include "pyscript/proxy_c.h"

/******************************************************************************/
// pyodide.ffi module

EM_JS(int, js_callable_proxy, (mp_obj_t callable, uint32_t *out), {
    const callback = function(js0, js1) {
        console.log('js_callable_proxy', js0, js1);
        let out2 = 0;
        if (js0 !== undefined) {
            out2 = _malloc(3 * 4);
            convert_js_to_mp_obj_jsside(js0, out2);
        }
        const out3 = _malloc(3 * 4);
        ccall("mp_obj_jsobj_callback_proxy", "number", ["pointer", "pointer", "pointer"], [callable, out2, out3]);
        if (out2 != 0) {
            _free(out2);
        }
        const ret = convert_mp_to_js_obj_jsside(out3);
        _free(out3);
        return ret;
    };
    convert_js_to_mp_obj_jsside(callback, out);
});

STATIC mp_obj_t mp_pyodide_ffi_create_proxy(mp_obj_t arg) {
    uint32_t out[3];
    js_callable_proxy(arg, out);
    return convert_js_to_mp_obj_cside(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_pyodide_ffi_create_proxy_obj, mp_pyodide_ffi_create_proxy);

STATIC const mp_rom_map_elem_t mp_module_pyodide_ffi_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ffi) },

    { MP_ROM_QSTR(MP_QSTR_create_proxy), MP_ROM_PTR(&mp_pyodide_ffi_create_proxy_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_pyodide_ffi_globals, mp_module_pyodide_ffi_globals_table);

const mp_obj_module_t mp_module_pyodide_ffi = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_pyodide_ffi_globals,
};

/******************************************************************************/
// pyodide module

STATIC const mp_rom_map_elem_t mp_module_pyodide_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_pyodide) },

    { MP_ROM_QSTR(MP_QSTR_ffi), MP_ROM_PTR(&mp_module_pyodide_ffi) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_pyodide_globals, mp_module_pyodide_globals_table);

const mp_obj_module_t mp_module_pyodide = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_pyodide_globals,
};

MP_REGISTER_MODULE(MP_QSTR_pyodide, mp_module_pyodide);
