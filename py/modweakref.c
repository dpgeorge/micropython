/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Damien P. George
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

#include "py/runtime.h"

#define PTR_TO_INT_OBJ(ptr) (MP_OBJ_NEW_SMALL_INT(((uintptr_t)ptr) >> 1))
#define PTR_FROM_INT_OBJ(obj) ((void *)(MP_OBJ_SMALL_INT_VALUE((obj)) << 1))

typedef struct _mp_obj_ref_t {
    mp_obj_base_t base;
    mp_obj_t obj;
    mp_obj_t callback;
    mp_obj_t ref_next;
} mp_obj_ref_t;

void mp_weakref_about_to_be_freed(void *ptr) {
    mp_obj_t idx = PTR_TO_INT_OBJ(ptr);
    mp_map_elem_t *elem = mp_map_lookup(&MP_STATE_VM(mp_weakref_map), idx, MP_MAP_LOOKUP);
    if (elem != NULL) {
        // Mark element as being freed.
        elem->key = mp_const_none;
    }
}

void mp_weakref_sweep(void) {
    mp_map_t *map = &MP_STATE_VM(mp_weakref_map);
    for (size_t i = 0; i < map->alloc; i++) {
        if (map->table[i].key == mp_const_none) {
            --map->used;
            map->table[i].key = MP_OBJ_SENTINEL;
            for (mp_obj_ref_t *ref = PTR_FROM_INT_OBJ(map->table[i].value); ref != NULL; ref = PTR_FROM_INT_OBJ(ref->ref_next)) { 
                ref->obj = mp_const_none; //
                if (ref->callback != mp_const_none) {
                    mp_call_function_1_protected(ref->callback, MP_OBJ_FROM_PTR(ref));
                }
            }
        }
    }
}

static mp_obj_t mp_obj_ref_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 2, false);

    if (!mp_obj_is_obj(args[0])) {
        mp_raise_TypeError(MP_ERROR_TEXT("can only weakref a true object"));
    }
    void *ptr = MP_OBJ_TO_PTR(args[0]);

    mp_obj_ref_t *self = mp_obj_malloc_with_finaliser(mp_obj_ref_t, type);
    self->obj = PTR_TO_INT_OBJ(ptr);
    if (n_args > 1) {
        self->callback = args[1];
    } else {
        self->callback = mp_const_none;
    }

    mp_map_elem_t *elem = mp_map_lookup(&MP_STATE_VM(mp_weakref_map), self->obj, MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
    self->ref_next = elem->value;
    elem->value = PTR_TO_INT_OBJ(self);

    return MP_OBJ_FROM_PTR(self);
}

static void mp_obj_ref_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_obj_ref_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<weakref at %p>", self);
}

static mp_obj_t mp_obj_ref_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_obj_ref_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->obj == mp_const_none) {
        return mp_const_none;
    } else {
        return MP_OBJ_FROM_PTR(PTR_FROM_INT_OBJ(self->obj));
    }
}

static mp_obj_t ref___del__(mp_obj_t self_in) {
    mp_obj_ref_t *self = MP_OBJ_TO_PTR(self_in);
    mp_map_elem_t *elem = mp_map_lookup(&MP_STATE_VM(mp_weakref_map), self->obj, MP_MAP_LOOKUP);
    if (elem != NULL) {
        for (mp_obj_t *link = &elem->value; PTR_FROM_INT_OBJ(*link) != NULL; link = &((mp_obj_ref_t *)PTR_FROM_INT_OBJ(*link))->ref_next) {
            if (self == PTR_FROM_INT_OBJ(*link)) {
                *link = self->ref_next;
                break;
            }
        }
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(ref___del___obj, ref___del__);

static const mp_rom_map_elem_t ref_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&ref___del___obj) },
};
static MP_DEFINE_CONST_DICT(ref_locals_dict, ref_locals_dict_table);

static MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_ref,
    MP_QSTR_ref,
    MP_TYPE_FLAG_NONE,
    make_new, mp_obj_ref_make_new,
    print, mp_obj_ref_print,
    call, mp_obj_ref_call,
    locals_dict, &ref_locals_dict
    );

static const mp_rom_map_elem_t mp_module_weakref_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_weakref) },
    { MP_ROM_QSTR(MP_QSTR_ref), MP_ROM_PTR(&mp_type_ref) },
};
static MP_DEFINE_CONST_DICT(mp_module_weakref_globals, mp_module_weakref_globals_table);

const mp_obj_module_t mp_module_weakref = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_weakref_globals,
};

MP_REGISTER_ROOT_POINTER(mp_map_t mp_weakref_map);
MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_weakref, mp_module_weakref);
