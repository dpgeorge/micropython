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
#include <stdlib.h>

#include "emscripten.h"
#include "py/objmodule.h"
#include "py/runtime.h"
#include "proxy_c.h"

// proxy value number of items
#define PVN (3)

EM_JS(bool, load_global, (const char *str, uint32_t *out), {
    let s = UTF8ToString(str);
    if (s in globalThis) {
        let value = globalThis[s];
        if (typeof value == "function") {
            value = value.bind(globalThis);
        }
        convert_js_to_mp_obj_jsside(value, out);
        return true;
    } else {
        return false;
    }
});

EM_JS(bool, lookup_attr, (int jsref, const char *str, uint32_t *out), {
    const base = proxy_js_ref[jsref];
    const attr = UTF8ToString(str);
    //console.debug("lookup_attr", jsref, base, attr);
    if (attr in base) {
        let value = base[attr];
        if (typeof value == "function") {
            value = value.bind(base);
        }
        convert_js_to_mp_obj_jsside(value, out);
        return true;
    } else {
        return false;
    }
});

EM_JS(void, store_attr, (int jsref, const char *attr_ptr, uint32_t *value_ref), {
    const attr = UTF8ToString(attr_ptr);
    const value = convert_mp_to_js_obj_jsside(value_ref);
    //console.debug("store_attr", proxy_js_ref[jsref], attr, value);
    proxy_js_ref[jsref][attr] = value;
});

EM_JS(void, call0, (int f_ref, uint32_t *out), {
    let f = proxy_js_ref[f_ref];
    let ret = f();
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(int, call1, (int f_ref, uint32_t *a0, uint32_t *out), {
    const a0_js = convert_mp_to_js_obj_jsside(a0);
    const ret = proxy_js_ref[f_ref](a0_js);
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(void, call0_kwarg, (int f_ref, uint32_t n_kw, uint32_t *key, uint32_t *value, uint32_t *out), {
    let f = proxy_js_ref[f_ref];
    let a = {};
    for (let i = 0; i < n_kw; ++i) {
        let k = UTF8ToString(getValue(key + i * 4, "i32"));
        let v = convert_mp_to_js_obj_jsside(value + i * 3 * 4);
        a[k] = v;
    }
    let ret = f(a);
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(void, call1_kwarg, (int f_ref, uint32_t *arg0, uint32_t n_kw, uint32_t *key, uint32_t *value, uint32_t *out), {
    let f = proxy_js_ref[f_ref];
    let a0 = convert_mp_to_js_obj_jsside(arg0);
    let a = {};
    for (let i = 0; i < n_kw; ++i) {
        let k = UTF8ToString(getValue(key + i * 4, "i32"));
        let v = convert_mp_to_js_obj_jsside(value + i * 3 * 4);
        a[k] = v;
    }
    let ret = f(a0, a);
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(int, call2, (int f_ref, uint32_t *a0, uint32_t *a1, uint32_t *out), {
    const a0_js = convert_mp_to_js_obj_jsside(a0);
    const a1_js = convert_mp_to_js_obj_jsside(a1);
    const ret = proxy_js_ref[f_ref](a0_js, a1_js);
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(void, js_reflect_construct, (int f_ref, uint32_t n_args, uint32_t *args, uint32_t *out), {
    const f = proxy_js_ref[f_ref];
    const as = [];
    for (let i = 0; i < n_args; ++i) {
        as.push(convert_mp_to_js_obj_jsside(args + i * 4));
    }
    const ret = Reflect.construct(f, as);
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(int, js_get_len, (int f_ref), {
    return proxy_js_ref[f_ref].length;
});

EM_JS(void, js_subscr_int, (int f_ref, int idx, uint32_t *out), {
    const ret = proxy_js_ref[f_ref][idx];
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(void, js_subscr_load, (int f_ref, uint32_t *index_ref, uint32_t *out), {
    const target = proxy_js_ref[f_ref];
    const index = python_index_semantics(target, convert_mp_to_js_obj_jsside(index_ref));
    const ret = target[index];
    convert_js_to_mp_obj_jsside(ret, out);
});

EM_JS(void, js_subscr_store, (int f_ref, uint32_t *idx, uint32_t *value), {
    proxy_js_ref[f_ref][convert_mp_to_js_obj_jsside(idx)] = convert_mp_to_js_obj_jsside(value);
});

typedef struct _mp_obj_jsobj_t {
    mp_obj_base_t base;
    int ref;
} mp_obj_jsobj_t;

STATIC const mp_obj_type_t mp_type_jsobj;

bool mp_obj_is_jsobj(mp_obj_t o) {
    return mp_obj_get_type(o) == &mp_type_jsobj;
}

int mp_obj_jsobj_get_ref(mp_obj_t o) {
    mp_obj_jsobj_t *self = MP_OBJ_TO_PTR(o);
    return self->ref;
}

void mp_obj_jsobj_callback_proxy(mp_obj_t obj, uint32_t *arg, uint32_t *out) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t ret;
        if (arg == NULL) {
            ret = mp_call_function_0(obj);
        } else {
            mp_obj_t arg_obj = convert_js_to_mp_obj_cside(arg);
            ret = mp_call_function_1(obj, arg_obj);
        }
        nlr_pop();
        convert_mp_to_js_obj_cside(ret, out);
    } else {
        convert_mp_to_js_exc_cside(nlr.ret_val, out);
    }
}

STATIC void jsobj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_obj_jsobj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<jsobj %d>", self->ref);
}

STATIC mp_obj_t jsobj_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_obj_jsobj_t *self = MP_OBJ_TO_PTR(self_in);

    if (n_kw == 0) {
        mp_arg_check_num(n_args, n_kw, 0, 2, false);
    } else {
        mp_arg_check_num(n_args, n_kw, 0, 1, true);
        uint32_t key[n_kw];
        uint32_t value[PVN * n_kw];
        for (int i = 0; i < n_kw; ++i) {
            key[i] = (uintptr_t)mp_obj_str_get_str(args[n_args + i * 2]);
            convert_mp_to_js_obj_cside(args[n_args + i * 2 + 1], &value[i * PVN]);
        }
        uint32_t out[3];
        if (n_args == 0) {
            call0_kwarg(self->ref, n_kw, key, value, out);
        } else {
            // n_args == 1
            uint32_t arg0[PVN];
            convert_mp_to_js_obj_cside(args[0], arg0);
            call1_kwarg(self->ref, arg0, n_kw, key, value, out);
        }
        return convert_js_to_mp_obj_cside(out);
    }

    if (n_args == 0) {
        uint32_t out[3];
        call0(self->ref, out);
        return convert_js_to_mp_obj_cside(out);
    } else if (n_args == 1) {
        uint32_t arg0[PVN];
        uint32_t out[PVN];
        convert_mp_to_js_obj_cside(args[0], arg0);
        call1(self->ref, arg0, out);
        return convert_js_to_mp_obj_cside(out);
    } else {
        uint32_t arg0[PVN];
        convert_mp_to_js_obj_cside(args[0], arg0);
        uint32_t arg1[PVN];
        convert_mp_to_js_obj_cside(args[1], arg1);
        uint32_t out[3];
        call2(self->ref, arg0, arg1, out);
        return convert_js_to_mp_obj_cside(out);
    }
}

STATIC mp_obj_t jsobj_reflect_construct(size_t n_args, const mp_obj_t *args) {
    int arg0 = mp_obj_jsobj_get_ref(args[0]);
    n_args -= 1;
    args += 1;
    uint32_t args_conv[n_args];
    for (unsigned int i = 0; i < n_args; ++i) {
        convert_mp_to_js_obj_cside(args[i], &args_conv[i * PVN]);
    }
    uint32_t out[3];
    js_reflect_construct(arg0, n_args, args_conv, out);
    return convert_js_to_mp_obj_cside(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(jsobj_reflect_construct_obj, 1, jsobj_reflect_construct);

STATIC mp_obj_t jsobj_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
    mp_obj_jsobj_t *self = MP_OBJ_TO_PTR(self_in);
    if (value == MP_OBJ_SENTINEL) {
        // Load subscript.
        uint32_t idx[PVN], out[PVN];
        convert_mp_to_js_obj_cside(index, idx);
        js_subscr_load(self->ref, idx, out);
        return convert_js_to_mp_obj_cside(out);
    } else if (value == MP_OBJ_NULL) {
        // Delete subscript.
        return MP_OBJ_NULL; // not supported
    } else {
        // Store subscript.
        uint32_t idx[PVN], val[PVN];
        convert_mp_to_js_obj_cside(index, idx);
        convert_mp_to_js_obj_cside(value, val);
        js_subscr_store(self->ref, idx, val);
        return mp_const_none;
    }
}

STATIC void jsobj_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    mp_obj_jsobj_t *self = MP_OBJ_TO_PTR(self_in);
    if (dest[0] == MP_OBJ_NULL) {
        // Load attribute.
        uint32_t out[PVN];
        if (lookup_attr(self->ref, qstr_str(attr), out)) {
            dest[0] = convert_js_to_mp_obj_cside(out);
        }
        if (attr == MP_QSTR_new) {
            // Special case to handle construction of JS objects.
            // JS objects don't have a ".new" attribute, doing "Obj.new" is a Pyodide idiom for "new Obj".
            // It translates to the JavaScript "Reflect.construct(Obj, Array(...args))".
            dest[0] = MP_OBJ_FROM_PTR(&jsobj_reflect_construct_obj);
            dest[1] = self_in;
            return;
        }
    } else if (dest[1] == MP_OBJ_NULL) {
        // Delete attribute.
    } else {
        // Store attribute.
        uint32_t value[PVN];
        convert_mp_to_js_obj_cside(dest[1], value);
        store_attr(self->ref, qstr_str(attr), value);
        dest[0] = MP_OBJ_NULL;
    }
}

/******************************************************************************/
// jsobj iterator

typedef struct _jsobj_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    int ref;
    uint16_t cur;
    uint16_t len;
} jsobj_it_t;

STATIC mp_obj_t jsobj_it_iternext(mp_obj_t self_in) {
    jsobj_it_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->cur < self->len) {
        uint32_t out[3];
        js_subscr_int(self->ref, self->cur, out);
        self->cur += 1;
        return convert_js_to_mp_obj_cside(out);
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

STATIC mp_obj_t jsobj_getiter(mp_obj_t o_in, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(jsobj_it_t) <= sizeof(mp_obj_iter_buf_t));
    jsobj_it_t *o = (jsobj_it_t *)iter_buf;
    o->base.type = &mp_type_polymorph_iter;
    o->iternext = jsobj_it_iternext;
    o->ref = mp_obj_jsobj_get_ref(o_in);
    o->cur = 0;
    o->len = js_get_len(o->ref);
    return MP_OBJ_FROM_PTR(o);
}

/******************************************************************************/

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_jsobj,
    MP_QSTR_jsobj,
    MP_TYPE_FLAG_ITER_IS_GETITER,
    print, jsobj_print,
    call, jsobj_call,
    attr, jsobj_attr,
    subscr, jsobj_subscr,
    iter, jsobj_getiter
    );

mp_obj_t mp_obj_new_jsobj(int ref) {
    mp_obj_jsobj_t *o = mp_obj_malloc(mp_obj_jsobj_t, &mp_type_jsobj);
    o->ref = ref;
    return MP_OBJ_FROM_PTR(o);
}

STATIC void mp_module_js_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    if (dest[0] != MP_OBJ_NULL) {
        // Only load supported.
        return;
    }

    uint32_t out[3];
    if (!load_global(qstr_str(attr), out)) {
        return;
    }

    dest[0] = convert_js_to_mp_obj_cside(out);
}

STATIC const mp_rom_map_elem_t mp_module_js_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_js) },

    MP_MODULE_ATTR_DELEGATION_ENTRY(&mp_module_js_attr),
};
STATIC MP_DEFINE_CONST_DICT(mp_module_js_globals, mp_module_js_globals_table);

const mp_obj_module_t mp_module_js = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_js_globals,
};

MP_REGISTER_MODULE(MP_QSTR_js, mp_module_js);
