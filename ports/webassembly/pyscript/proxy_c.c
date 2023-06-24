#include <stdlib.h>
#include <string.h>

#include "py/runtime.h"
#include "proxy_c.h"

void proxy_c_init(void) {
    MP_STATE_PORT(proxy_c_ref) = mp_obj_new_list(0, NULL);
    mp_obj_list_append(MP_STATE_PORT(proxy_c_ref), MP_OBJ_NULL);
}

MP_REGISTER_ROOT_POINTER(mp_obj_t proxy_c_ref);

mp_obj_t convert_js_to_mp_obj_cside(uint32_t *value) {
    if (value[0] == 1) {
        return mp_const_none;
    } else if (value[0] == 2) {
        return mp_obj_new_bool(value[1]);
    } else if (value[0] == 3) {
        return mp_obj_new_int(value[1]);
    } else if (value[0] == 4) {
        mp_obj_t s = mp_obj_new_str((void *)value[2], value[1]);
        free((void *)value[2]);
        return s;
    } else {
        return mp_obj_new_jsobj(value[1]);
    }
}

void convert_mp_to_js_obj_cside(mp_obj_t obj, uint32_t *out) {
    uint32_t kind;
    if (obj == MP_OBJ_NULL) {
        kind = 0;
    } else if (obj == mp_const_none) {
        kind = 1;
    } else if (mp_obj_is_bool(obj)) {
        kind = 2;
        out[1] = mp_obj_is_true(obj);
    } else if (mp_obj_is_int(obj)) {
        kind = 3;
        out[1] = mp_obj_get_int_truncated(obj); // TODO support big int
    } else if (mp_obj_is_str(obj)) {
        kind = 4;
        size_t len;
        const char *str = mp_obj_str_get_data(obj, &len);
        out[1] = len;
        out[2] = (uintptr_t)str;
    } else if (mp_obj_is_jsobj(obj)) {
        kind = 7;
        out[1] = mp_obj_jsobj_get_ref(obj);
    } else {
        if (mp_obj_is_callable(obj)) {
            kind = 5;
        } else {
            kind = 6;
        }
        size_t id = ((mp_obj_list_t *)MP_OBJ_TO_PTR(MP_STATE_PORT(proxy_c_ref)))->len;
        mp_obj_list_append(MP_STATE_PORT(proxy_c_ref), obj);
        out[1] = id;
    }
    out[0] = kind;
}

void convert_mp_to_js_exc_cside(void *exc, uint32_t *out) {
    out[0] = -1;
    vstr_t vstr;
    mp_print_t print;
    vstr_init_print(&vstr, 64, &print);
    //mp_obj_print_helper(&print, MP_OBJ_FROM_PTR(exc), PRINT_EXC);
    mp_obj_print_exception(&print, MP_OBJ_FROM_PTR(exc));
    mp_obj_print_exception(MICROPY_ERROR_PRINTER, MP_OBJ_FROM_PTR(exc));
    char *s = malloc(vstr_len(&vstr) + 1);
    memcpy(s, vstr_str(&vstr), vstr_len(&vstr));
    out[1] = vstr_len(&vstr);
    out[2] = (uintptr_t)s;
    vstr_clear(&vstr);
}

void proxy_c_to_js_call(uint32_t c_ref, uint32_t n_args, uint32_t *args_value, uint32_t *out) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t args[4] = { mp_const_none, mp_const_none, mp_const_none, mp_const_none };
        for (size_t i = 0; i < n_args; ++i) {
            args[i] = convert_js_to_mp_obj_cside(args_value + i * 3);
        }
        mp_obj_t obj = ((mp_obj_list_t *)MP_OBJ_TO_PTR(MP_STATE_PORT(proxy_c_ref)))->items[c_ref];
        mp_obj_t member = mp_call_function_n_kw(obj, n_args, 0, args);
        nlr_pop();
        convert_mp_to_js_obj_cside(member, out);
    } else {
        // uncaught exception
        convert_mp_to_js_exc_cside(nlr.ret_val, out);
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(mp_obj_dict_store_obj, mp_obj_dict_store);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mp_obj_dict_delete_obj, mp_obj_dict_delete);

void proxy_c_to_js_lookup_attr(uint32_t c_ref, const char *attr, uint32_t *out) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t obj = ((mp_obj_list_t *)MP_OBJ_TO_PTR(MP_STATE_PORT(proxy_c_ref)))->items[c_ref];
        qstr qst = qstr_from_str(attr);
        #if 0
        mp_printf(MICROPY_ERROR_PRINTER, "lookup %s from ", attr);
        mp_obj_print_helper(MICROPY_ERROR_PRINTER, obj, PRINT_REPR);
        mp_printf(MICROPY_ERROR_PRINTER, "\n");
        #endif
        mp_obj_t member;
        if (qst == MP_QSTR_set && mp_obj_is_dict_or_ordereddict(obj)) {
            member = mp_obj_new_bound_meth(MP_OBJ_FROM_PTR(&mp_obj_dict_store_obj), obj);
        } else if (qst == MP_QSTR_delete && mp_obj_is_dict_or_ordereddict(obj)) {
            member = mp_obj_new_bound_meth(MP_OBJ_FROM_PTR(&mp_obj_dict_delete_obj), obj);
        } else if (qst == MP_QSTR_then && mp_obj_is_type(obj, &mp_type_gen_instance)) {
            member = mp_obj_new_bound_meth(MP_OBJ_FROM_PTR(&then_obj), obj);
        } else {
            member = mp_load_attr(obj, qstr_from_str(attr));
        }
        nlr_pop();
        return convert_mp_to_js_obj_cside(member, out);
    } else {
        // uncaught exception
        return convert_mp_to_js_exc_cside(nlr.ret_val, out);
    }
}
