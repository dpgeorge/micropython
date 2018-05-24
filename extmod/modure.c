/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Paul Sokolovsky
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
#include <assert.h>
#include <string.h>

#include "py/runtime.h"
#include "py/binary.h"
#include "py/objstr.h"
#include "py/stackctrl.h"

#if MICROPY_PY_URE

#define re1_5_stack_chk() MP_STACK_CHECK()

#include "re1.5/re1.5.h"

#define FLAG_DEBUG 0x1000

typedef struct _mp_obj_re_t {
    mp_obj_base_t base;
    ByteProg re;
} mp_obj_re_t;

typedef struct _mp_obj_match_t {
    mp_obj_base_t base;
    int num_matches;
    mp_obj_t str;
    const char *caps[0];
} mp_obj_match_t;


STATIC void match_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    mp_obj_match_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<match num=%d>", self->num_matches);
}

STATIC mp_obj_t match_group(mp_obj_t self_in, mp_obj_t no_in) {
    mp_obj_match_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t no = mp_obj_get_int(no_in);
    if (no < 0 || no >= self->num_matches) {
        nlr_raise(mp_obj_new_exception_arg1(&mp_type_IndexError, no_in));
    }

    const char *start = self->caps[no * 2];
    if (start == NULL) {
        // no match for this group
        return mp_const_none;
    }
    return mp_obj_new_str_of_type(mp_obj_get_type(self->str),
        (const byte*)start, self->caps[no * 2 + 1] - start);
}
MP_DEFINE_CONST_FUN_OBJ_2(match_group_obj, match_group);

#if MICROPY_PY_URE_MATCH_GROUPS

STATIC mp_obj_t match_groups(mp_obj_t self_in) {
    mp_obj_match_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->num_matches <= 1) {
        return mp_const_empty_tuple;
    }
    mp_obj_tuple_t *groups = MP_OBJ_TO_PTR(mp_obj_new_tuple(self->num_matches - 1, NULL));
    for (ssize_t no = 1; no < self->num_matches; no++) {
        const char *start = self->caps[no * 2];
        if (start == NULL) {
            // no match for this group
            groups->items[no - 1] = mp_const_none;
        } else {
            groups->items[no - 1] = mp_obj_new_str_of_type(mp_obj_get_type(self->str),
                (const byte*)start, self->caps[no * 2 + 1] - start);
        }
    }
    return MP_OBJ_FROM_PTR(groups);
}
MP_DEFINE_CONST_FUN_OBJ_1(match_groups_obj, match_groups);

#endif

#if MICROPY_PY_URE_MATCH_SPAN_START_END

STATIC mp_obj_t match_span(size_t n_args, const mp_obj_t *args) {
    mp_obj_match_t *self = MP_OBJ_TO_PTR(args[0]);

    mp_int_t no = 0;
    if (n_args == 2) {
        no = mp_obj_get_int(args[1]);
        if (no < 0 || no >= self->num_matches) {
            nlr_raise(mp_obj_new_exception_arg1(&mp_type_IndexError, args[1]));
        }
    }

    mp_int_t s = -1;
    mp_int_t e = -1;
    const char *start = self->caps[no * 2];
    if (start != NULL) {
        // have a match for this group
        const char *begin = mp_obj_str_get_str(self->str);
        s = start - begin;
        e = self->caps[no * 2 + 1] - begin;
    }

    mp_obj_t tuple[2] = {mp_obj_new_int(s), mp_obj_new_int(e)};
    return mp_obj_new_tuple(2, tuple);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(match_span_obj, 1, 2, match_span);

STATIC mp_obj_t match_start(size_t n_args, const mp_obj_t *args) {
    mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(match_span(n_args, args));
    return tuple->items[0];
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(match_start_obj, 1, 2, match_start);

STATIC mp_obj_t match_end(size_t n_args, const mp_obj_t *args) {
    mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(match_span(n_args, args));
    return tuple->items[1];
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(match_end_obj, 1, 2, match_end);

#endif

STATIC const mp_rom_map_elem_t match_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_group), MP_ROM_PTR(&match_group_obj) },
    #if MICROPY_PY_URE_MATCH_GROUPS
    { MP_ROM_QSTR(MP_QSTR_groups), MP_ROM_PTR(&match_groups_obj) },
    #endif
    #if MICROPY_PY_URE_MATCH_SPAN_START_END
    { MP_ROM_QSTR(MP_QSTR_span), MP_ROM_PTR(&match_span_obj) },
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&match_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_end), MP_ROM_PTR(&match_end_obj) },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(match_locals_dict, match_locals_dict_table);

STATIC const mp_obj_type_t match_type = {
    { &mp_type_type },
    .name = MP_QSTR_match,
    .print = match_print,
    .locals_dict = (void*)&match_locals_dict,
};

STATIC void re_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    mp_obj_re_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<re %p>", self);
}

STATIC mp_obj_t ure_exec(bool is_anchored, uint n_args, const mp_obj_t *args) {
    (void)n_args;
    mp_obj_re_t *self = MP_OBJ_TO_PTR(args[0]);
    Subject subj;
    size_t len;
    subj.begin = mp_obj_str_get_data(args[1], &len);
    subj.end = subj.begin + len;
    int caps_num = (self->re.sub + 1) * 2;
    mp_obj_match_t *match = m_new_obj_var(mp_obj_match_t, char*, caps_num);
    // cast is a workaround for a bug in msvc: it treats const char** as a const pointer instead of a pointer to pointer to const char
    memset((char*)match->caps, 0, caps_num * sizeof(char*));
    int res = re1_5_recursiveloopprog(&self->re, &subj, match->caps, caps_num, is_anchored);
    if (res == 0) {
        m_del_var(mp_obj_match_t, char*, caps_num, match);
        return mp_const_none;
    }

    match->base.type = &match_type;
    match->num_matches = caps_num / 2; // caps_num counts start and end pointers
    match->str = args[1];
    return MP_OBJ_FROM_PTR(match);
}

STATIC mp_obj_t re_match(size_t n_args, const mp_obj_t *args) {
    return ure_exec(true, n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(re_match_obj, 2, 4, re_match);

STATIC mp_obj_t re_search(size_t n_args, const mp_obj_t *args) {
    return ure_exec(false, n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(re_search_obj, 2, 4, re_search);

STATIC mp_obj_t re_split(size_t n_args, const mp_obj_t *args) {
    mp_obj_re_t *self = MP_OBJ_TO_PTR(args[0]);
    Subject subj;
    size_t len;
    const mp_obj_type_t *str_type = mp_obj_get_type(args[1]);
    subj.begin = mp_obj_str_get_data(args[1], &len);
    subj.end = subj.begin + len;
    int caps_num = (self->re.sub + 1) * 2;

    int maxsplit = 0;
    if (n_args > 2) {
        maxsplit = mp_obj_get_int(args[2]);
    }

    mp_obj_t retval = mp_obj_new_list(0, NULL);
    const char **caps = mp_local_alloc(caps_num * sizeof(char*));
    while (true) {
        // cast is a workaround for a bug in msvc: it treats const char** as a const pointer instead of a pointer to pointer to const char
        memset((char**)caps, 0, caps_num * sizeof(char*));
        int res = re1_5_recursiveloopprog(&self->re, &subj, caps, caps_num, false);

        // if we didn't have a match, or had an empty match, it's time to stop
        if (!res || caps[0] == caps[1]) {
            break;
        }

        mp_obj_t s = mp_obj_new_str_of_type(str_type, (const byte*)subj.begin, caps[0] - subj.begin);
        mp_obj_list_append(retval, s);
        if (self->re.sub > 0) {
            mp_raise_NotImplementedError("Splitting with sub-captures");
        }
        subj.begin = caps[1];
        if (maxsplit > 0 && --maxsplit == 0) {
            break;
        }
    }
    // cast is a workaround for a bug in msvc (see above)
    mp_local_free((char**)caps);

    mp_obj_t s = mp_obj_new_str_of_type(str_type, (const byte*)subj.begin, subj.end - subj.begin);
    mp_obj_list_append(retval, s);
    return retval;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(re_split_obj, 2, 3, re_split);

#if MICROPY_PY_URE_SUB
// str_to_int ... from objstr.c
STATIC int str_to_int(const char *str, int *num) {
    const char *s = str;
    if ('0' <= *s && *s <= '9') {
        *num = 0;
        do {
            *num = *num * 10 + (*s - '0');
            s++;
        } while ('0' <= *s && *s <= '9');
    }
    return s - str;
}

STATIC mp_obj_t ure_exec_sub(mp_obj_re_t *self, mp_obj_t replace, mp_obj_t where, mp_int_t count, mp_int_t flags) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(where, &bufinfo, MP_BUFFER_READ);
    bool debug = (flags & FLAG_DEBUG);
    (void)debug;
    Subject subj;
    subj.begin = bufinfo.buf;
    subj.end = subj.begin + bufinfo.len;
    int caps_num = (self->re.sub + 1) * 2;
    int pre_m_l = -1, post_m_l = -1;
    int num_sub = 0;
    const char* where_str = subj.begin;

    vstr_t *vstr_return = vstr_new(0);
    for (;;) {
        mp_obj_match_t *match = m_new_obj_var(mp_obj_match_t, char*, caps_num);
        // cast is a workaround for a bug in msvc: it treats const char** as a const pointer instead of a pointer to pointer to const char
        memset((char*)match->caps, 0, caps_num * sizeof(char*));
        int res = re1_5_recursiveloopprog(&self->re, &subj, match->caps, caps_num, false);

        // if we didn't have a match, or had an empty match, it's time to stop
        if (!res || match->caps[0] == match->caps[1]) {
            break;
        }

        // process match

        match->base.type = &match_type;
        match->num_matches = caps_num / 2; // caps_num counts start and end pointers
        match->str = where;

        const char* replace_str = mp_obj_str_get_str((mp_obj_is_callable(replace) ? mp_call_function_1(replace, MP_OBJ_FROM_PTR(match)) : replace));

        // add pre-match string
        pre_m_l = (match->caps[0] - subj.begin);
        if (pre_m_l != -1) {
            vstr_add_strn(vstr_return, subj.begin, pre_m_l);
        }

        vstr_t *vstr_repl = vstr_new(0);
        vstr_add_str(vstr_repl, replace_str);
        const char *repl_p = vstr_null_terminated_str(vstr_repl);
        do {
            const char *group = NULL, *start_group = NULL, *end_group = NULL;
            int match_no = -1, is_group_number = -1;
            if (*repl_p == '\\') {
                start_group = repl_p;
                group = ++repl_p;

                if (group != 0 && unichar_isdigit(*group)) {
                    // search group with syntax "\number"
                    const char *value = group;
                    int value_l = str_to_int(value, &match_no);
                    if (match_no > -1 && match_no < match->num_matches) {
                        is_group_number = 0;
                        end_group = value += value_l;
                    }
                }

                if (match_no > -1 && is_group_number > -1 && start_group != NULL && end_group != NULL) {
                    const char *start_match = match->caps[match_no * 2];
                    if (start_match == NULL) {
                        // no match for this group
                        return where;
                    }

                    size_t mg_l = (match->caps[match_no * 2 + 1] - start_match);
                    size_t gv_l = (end_group - start_group);

                    if (gv_l < mg_l) {
                        vstr_add_len(vstr_repl, (mg_l - gv_l));
                        repl_p += gv_l;
                    } else if (gv_l > mg_l) {
                        vstr_cut_out_bytes(vstr_repl, (start_group - replace_str), (gv_l - mg_l));
                        repl_p -= mg_l;
                    }

                    // replace the substring matched by group
                    memmove((char *)(start_group + mg_l), (start_group + gv_l), strlen(start_group));
                    memcpy((char *)(start_group), start_match, mg_l);
                }
            }
        } while (*(++repl_p) != 0);

        // add replace
        vstr_add_str(vstr_return, vstr_str(vstr_repl));

        // post-match string
        post_m_l = (match->caps[1] - where_str);

        vstr_free(vstr_repl);

        num_sub++;

        subj.begin = match->caps[1];
        subj.end = subj.begin + strlen(match->caps[1]);

        if (count > 0 && --count == 0) {
            break;
        }

        m_del_var(mp_obj_match_t, char*, match->num_matches, match);
    }

    // add post-match string
    if (post_m_l != -1) {
        vstr_add_str(vstr_return, &where_str[post_m_l]);
    }

    const mp_obj_type_t *str_type = mp_obj_get_type(where);
    return (!num_sub ? where : mp_obj_new_str_from_vstr(str_type, vstr_return));
}

STATIC mp_obj_t re_sub_helper(mp_obj_t self, size_t n_args, const mp_obj_t *args) {
    mp_obj_t replace = args[1];
    mp_obj_t where = args[2];
    mp_int_t count = 0;
    mp_int_t flags = 0;
    if (n_args > 3) {
        count = mp_obj_get_int(args[3]);
        if (n_args > 4) {
            flags = mp_obj_get_int(args[4]);
        }
    }
    return ure_exec_sub(MP_OBJ_TO_PTR(self), replace, where, count, flags);
}

STATIC mp_obj_t re_sub(size_t n_args, const mp_obj_t *args) {
    return re_sub_helper(args[0], n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(re_sub_n_obj, 3, 5, re_sub);
#endif

STATIC const mp_rom_map_elem_t re_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_match), MP_ROM_PTR(&re_match_obj) },
    { MP_ROM_QSTR(MP_QSTR_search), MP_ROM_PTR(&re_search_obj) },
    { MP_ROM_QSTR(MP_QSTR_split), MP_ROM_PTR(&re_split_obj) },
    #if MICROPY_PY_URE_SUB
    { MP_ROM_QSTR(MP_QSTR_sub), MP_ROM_PTR(&re_sub_n_obj) },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(re_locals_dict, re_locals_dict_table);

STATIC const mp_obj_type_t re_type = {
    { &mp_type_type },
    .name = MP_QSTR_ure,
    .print = re_print,
    .locals_dict = (void*)&re_locals_dict,
};

STATIC mp_obj_t mod_re_compile(size_t n_args, const mp_obj_t *args) {
    const char *re_str = mp_obj_str_get_str(args[0]);
    int size = re1_5_sizecode(re_str);
    if (size == -1) {
        goto error;
    }
    mp_obj_re_t *o = m_new_obj_var(mp_obj_re_t, char, size);
    o->base.type = &re_type;
    int flags = 0;
    if (n_args > 1) {
        flags = mp_obj_get_int(args[1]);
    }
    int error = re1_5_compilecode(&o->re, re_str);
    if (error != 0) {
error:
        mp_raise_ValueError("Error in regex");
    }
    if (flags & FLAG_DEBUG) {
        re1_5_dumpcode(&o->re);
    }
    return MP_OBJ_FROM_PTR(o);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_re_compile_obj, 1, 2, mod_re_compile);

STATIC mp_obj_t mod_re_exec(bool is_anchored, uint n_args, const mp_obj_t *args) {
    (void)n_args;
    mp_obj_t self = mod_re_compile(1, args);

    const mp_obj_t args2[] = {self, args[1]};
    mp_obj_t match = ure_exec(is_anchored, 2, args2);
    return match;
}

STATIC mp_obj_t mod_re_match(size_t n_args, const mp_obj_t *args) {
    return mod_re_exec(true, n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_re_match_obj, 2, 4, mod_re_match);

STATIC mp_obj_t mod_re_search(size_t n_args, const mp_obj_t *args) {
    return mod_re_exec(false, n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_re_search_obj, 2, 4, mod_re_search);

#if MICROPY_PY_URE_SUB
STATIC mp_obj_t mod_re_sub(size_t n_args, const mp_obj_t *args) {
    mp_obj_t self = mod_re_compile(1, args);
    return re_sub_helper(self, n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_re_sub_obj, 3, 5, mod_re_sub);
#endif

STATIC const mp_rom_map_elem_t mp_module_re_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ure) },
    { MP_ROM_QSTR(MP_QSTR_compile), MP_ROM_PTR(&mod_re_compile_obj) },
    { MP_ROM_QSTR(MP_QSTR_match), MP_ROM_PTR(&mod_re_match_obj) },
    { MP_ROM_QSTR(MP_QSTR_search), MP_ROM_PTR(&mod_re_search_obj) },
    #if MICROPY_PY_URE_SUB
    { MP_ROM_QSTR(MP_QSTR_sub), MP_ROM_PTR(&mod_re_sub_obj) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_DEBUG), MP_ROM_INT(FLAG_DEBUG) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_re_globals, mp_module_re_globals_table);

const mp_obj_module_t mp_module_ure = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_re_globals,
};

// Source files #include'd here to make sure they're compiled in
// only if module is enabled by config setting.

#define re1_5_fatal(x) assert(!x)
#include "re1.5/compilecode.c"
#include "re1.5/dumpcode.c"
#include "re1.5/recursiveloop.c"
#include "re1.5/charclass.c"

#endif //MICROPY_PY_URE
