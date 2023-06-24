#include <string.h>

#include "py/compile.h"
#include "py/gc.h"
#include "py/runtime.h"
#include "pyscript/proxy_c.h"

static size_t dedent(const byte *text, size_t len) {
    size_t min_prefix = -1;
    size_t cur_prefix = 0;
    bool start_of_line = true;
    for (const byte *t = text; t < text + len; ++t) {
        if (*t == '\n') {
            start_of_line = true;
            cur_prefix = 0;
        } else if (start_of_line) {
            if (unichar_isspace(*t)) {
                ++cur_prefix;
            } else {
                if (cur_prefix < min_prefix) {
                    min_prefix = cur_prefix;
                }
                start_of_line = false;
            }
        }
    }
    return min_prefix;
}

typedef struct _mp_reader_mem_dedent_t {
    size_t free_len; // if >0 mem is freed on close by: m_free(beg, free_len)
    const byte *beg;
    const byte *cur;
    const byte *end;
    size_t dedent_prefix;
} mp_reader_mem_dedent_t;

static mp_uint_t mp_reader_mem_dedent_readbyte(void *data) {
    mp_reader_mem_dedent_t *reader = (mp_reader_mem_dedent_t *)data;
    if (reader->cur < reader->end) {
        byte c = *reader->cur++;
        if (c == '\n') {
            for (size_t i = 0; i < reader->dedent_prefix; ++i) {
                if (*reader->cur == '\n') {
                    break;
                }
                ++reader->cur;
            }
        }
        //mp_printf(MICROPY_ERROR_PRINTER, "CH<%c>\n", c);
        return c;
    } else {
        return MP_READER_EOF;
    }
}

static void mp_reader_mem_dedent_close(void *data) {
    mp_reader_mem_dedent_t *reader = (mp_reader_mem_dedent_t *)data;
    if (reader->free_len > 0) {
        m_del(char, (char *)reader->beg, reader->free_len);
    }
    m_del_obj(mp_reader_mem_dedent_t, reader);
}

void mp_reader_new_mem_dedent(mp_reader_t *reader, const byte *buf, size_t len, size_t free_len) {
    mp_reader_mem_dedent_t *rm = m_new_obj(mp_reader_mem_dedent_t);
    rm->free_len = free_len;
    rm->beg = buf;
    rm->cur = buf;
    rm->end = buf + len;
    rm->dedent_prefix = dedent(buf, len);
    //mp_printf(MICROPY_ERROR_PRINTER, "dedent %d\n", rm->dedent_prefix);
    reader->data = rm;
    reader->readbyte = mp_reader_mem_dedent_readbyte;
    reader->close = mp_reader_mem_dedent_close;
}

static mp_lexer_t *mp_lexer_new_from_str_len_dedent(qstr src_name, const char *str, size_t len, size_t free_len) {
    mp_reader_t reader;
    mp_reader_new_mem_dedent(&reader, (const byte *)str, len, free_len);
    return mp_lexer_new(src_name, reader);
}

void mp_js_register_js_module(const char *name, uint32_t *value) {
    mp_obj_t module_name = MP_OBJ_NEW_QSTR(qstr_from_str(name));
    mp_obj_t module = convert_js_to_mp_obj_cside(value);
    mp_map_t *mp_loaded_modules_map = &MP_STATE_VM(mp_loaded_modules_dict).map;
    mp_map_lookup(mp_loaded_modules_map, module_name, MP_MAP_LOOKUP_ADD_IF_NOT_FOUND)->value = module;
}

void mp_js_do_import(const char *name, uint32_t *out) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t ret = mp_import_name(qstr_from_str(name), mp_const_none, MP_OBJ_NEW_SMALL_INT(0));
        // Return the leaf of the import, eg for "a.b.c" return "c".
        const char *m = name;
        const char *n = name;
        for (;; ++n) {
            if (*n == '\0' || *n == '.') {
                if (m != name) {
                    ret = mp_load_attr(ret, qstr_from_strn(m, n - m));
                }
                m = n + 1;
                if (*n == '\0') {
                    break;
                }
            }
        }
        nlr_pop();
        convert_mp_to_js_obj_cside(ret, out);
    } else {
        // uncaught exception
        convert_mp_to_js_exc_cside(nlr.ret_val, out);
    }
}

void mp_js_do_exec(const char *src, uint32_t *out) {
    // Collect at the top-level, where there are no root pointers from stack/registers.
    gc_collect_start();
    gc_collect_end();

    mp_parse_input_kind_t input_kind = MP_PARSE_FILE_INPUT;
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        //mp_printf(MICROPY_ERROR_PRINTER, "run %s\n", src);
        mp_lexer_t *lex = mp_lexer_new_from_str_len_dedent(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, false);
        mp_obj_t ret = mp_call_function_0(module_fun);
        nlr_pop();
        convert_mp_to_js_obj_cside(ret, out);
    } else {
        // uncaught exception
        convert_mp_to_js_exc_cside(nlr.ret_val, out);
    }
}

void mp_js_do_exec_async(const char *src, uint32_t *out) {
    mp_compile_allow_top_level_await = true;
    mp_js_do_exec(src, out);
    mp_compile_allow_top_level_await = false;
}
