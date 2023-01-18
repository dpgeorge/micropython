/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Paul Sokolovsky
 * Copyright (c) 2021-2023 Damien P. George
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
#include <string.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"

#if MICROPY_PY_UZLIB

#include "lib/uzlib/uzlib.h"

#if 0 // print debugging info
#define DEBUG_printf DEBUG_printf
#else // don't print debugging info
#define DEBUG_printf(...) (void)0
#endif

#if MICROPY_PY_UZLIB_COMPRESS

// TODO: check this header
#define ZLIB_HEADER ("\x78\x9c")
#define ZLIB_HEADER_LEN (2)

#define GZIP_HEADER ("\x1f\x8b\x08\x00" "\x00\x00\x00\x00" "\x04\x03")
#define GZIP_HEADER_LEN (10)

typedef enum {
    OUT_RAW,
    OUT_ZLIB,
    OUT_GZIP,
} out_type_t;

typedef struct _mp_obj_compio_t {
    mp_obj_base_t base;
    mp_obj_t dest_stream;
    mp_obj_t hist_obj;
    size_t input_len;
    uint32_t input_checksum;
    out_type_t out_type;
    struct uzlib_lz77_state lz77;
} mp_obj_compio_t;

static inline void put_le32(char *buf, uint32_t value) {
    buf[0] = value & 0xff;
    buf[1] = value >> 8 & 0xff;
    buf[2] = value >> 16 & 0xff;
    buf[3] = value >> 24 & 0xff;
}

static inline void put_be32(char *buf, uint32_t value) {
    buf[3] = value & 0xff;
    buf[2] = value >> 8 & 0xff;
    buf[1] = value >> 16 & 0xff;
    buf[0] = value >> 24 & 0xff;
}

STATIC out_type_t parse_wbits(mp_obj_t wbits_arg, size_t *hist_len) {
    mp_int_t wbits = 8;
    if (wbits_arg != MP_OBJ_NULL) {
        wbits = mp_obj_get_int(wbits_arg);
    }
    if (wbits < 0) {
        *hist_len = 1 << -wbits;
        return OUT_RAW;
    } else if (wbits < 16) {
        *hist_len = 1 << wbits;
        return OUT_ZLIB;
    } else {
        *hist_len = 1 << (wbits - 16);
        return OUT_GZIP;
    }
}

STATIC void compio_out_byte(struct Outbuf *outbuf, uint8_t b) {
    mp_obj_compio_t *self = outbuf->dest_write_data;
    const mp_stream_p_t *stream = mp_get_stream(self->dest_stream);
    int err;
    mp_uint_t ret = stream->write(self->dest_stream, &b, 1, &err);
    if (ret == MP_STREAM_ERROR) {
        mp_raise_OSError(err);
    }
}

STATIC mp_obj_t compio_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 2, false);

    // Check that the input stream is a stream with write capabilities.
    mp_get_stream_raise(args[0], MP_STREAM_OP_WRITE);

    // Parse "wbits" argument.
    size_t hist_len;
    out_type_t out_type = parse_wbits(n_args > 1 ? args[1] : MP_OBJ_NULL, &hist_len);

    // Get or allocate the history buffer.
    mp_obj_t hist_obj = MP_OBJ_NULL;
    uint8_t *hist = NULL;
    if (n_args > 2) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_RW);
        if (bufinfo.len < hist_len) {
            hist_len = bufinfo.len;
        }
        hist_obj = args[2];
        hist = bufinfo.buf;
    }
    if (hist == NULL) {
        hist = m_new(uint8_t, hist_len);
    }

    // Initialise compression object and state.
    mp_obj_compio_t *self = m_new_obj(mp_obj_compio_t);
    self->base.type = type;
    self->dest_stream = args[0];
    self->hist_obj = hist_obj;
    self->input_len = 0;
    if (out_type == OUT_ZLIB) {
        self->input_checksum = 1; // ADLER32
    } else {
        self->input_checksum = ~0; // CRC32
    }
    self->out_type = out_type;
    uzlib_lz77_init(&self->lz77, hist, hist_len);
    self->lz77.outbuf.dest_write_data = self;
    self->lz77.outbuf.dest_write_cb = compio_out_byte;

    // Write header if needed.
    if (out_type != OUT_RAW) {
        const mp_stream_p_t *stream = mp_get_stream(self->dest_stream);
        int err;
        mp_uint_t ret;
        if (out_type == OUT_ZLIB) {
            ret = stream->write(self->dest_stream, ZLIB_HEADER, ZLIB_HEADER_LEN, &err);
        } else {
            ret = stream->write(self->dest_stream, GZIP_HEADER, GZIP_HEADER_LEN, &err);
        }
        if (ret == MP_STREAM_ERROR) {
            mp_raise_OSError(err);
        }
    }

    // Write starting block.
    zlib_start_block(&self->lz77.outbuf);

    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t compio_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mp_stream_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write_obj) },
};
STATIC MP_DEFINE_CONST_DICT(compio_locals_dict, compio_locals_dict_table);

STATIC mp_uint_t compio_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    mp_obj_compio_t *self = MP_OBJ_TO_PTR(self_in);
    self->input_len += size;
    if (self->out_type == OUT_ZLIB) {
        self->input_checksum = uzlib_adler32(buf, size, self->input_checksum);
    } else if (self->out_type == OUT_GZIP) {
        self->input_checksum = uzlib_crc32(buf, size, self->input_checksum);
    }
    uzlib_lz77_compress(&self->lz77, buf, size);
    return size;
}

STATIC mp_uint_t compio_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    mp_obj_compio_t *self = MP_OBJ_TO_PTR(self_in);

    if (request == MP_STREAM_CLOSE) {
        if (self->dest_stream != MP_OBJ_NULL) {
            zlib_finish_block(&self->lz77.outbuf);

            const mp_stream_p_t *stream = mp_get_stream(self->dest_stream);

            // Write footer if needed.
            if (self->out_type != OUT_RAW) {
                char footer[8];
                size_t footer_len;
                if (self->out_type == OUT_ZLIB) {
                    put_be32(&footer[0], self->input_checksum);
                    footer_len = 4;
                } else {
                    put_le32(&footer[0], ~self->input_checksum);
                    put_le32(&footer[4], self->input_len);
                    footer_len = 8;
                }
                mp_uint_t ret = stream->write(self->dest_stream, footer, footer_len, errcode);
                if (ret == MP_STREAM_ERROR) {
                    self->dest_stream = MP_OBJ_NULL;
                    return ret;
                }
            }

            // Don't close the stream (the caller may still want to write to it, or in
            // the case of BytesIO call getvalue), instead just free the reference to it.
            self->dest_stream = MP_OBJ_NULL;
        }
        return 0;
    } else {
        *errcode = MP_EINVAL;
        return MP_STREAM_ERROR;
    }
}

STATIC const mp_stream_p_t compio_stream_p = {
    .write = compio_write,
    .ioctl = compio_ioctl,
};

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    compio_type,
    MP_QSTR_CompIO,
    MP_TYPE_FLAG_NONE,
    make_new, compio_make_new,
    protocol, &compio_stream_p,
    locals_dict, &compio_locals_dict
    );

STATIC void compress_out_byte(struct Outbuf *outbuf, uint8_t byte) {
    vstr_add_byte(outbuf->dest_write_data, byte);
}

STATIC mp_obj_t mod_uzlib_compress(size_t n_args, const mp_obj_t *args) {
    // Parse "data" argument.
    mp_obj_t data = args[0];
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);

    // Parse "wbits" argument.
    size_t hist_len;
    out_type_t out_type = parse_wbits(n_args > 2 ? args[2] : MP_OBJ_NULL, &hist_len);

    // Initialise compression object and state.
    vstr_t vstr;
    vstr_init(&vstr, bufinfo.len / 8);
    struct uzlib_lz77_state lz77;
    uzlib_lz77_init(&lz77, NULL, hist_len);
    lz77.outbuf.dest_write_data = &vstr;
    lz77.outbuf.dest_write_cb = compress_out_byte;

    // Write header if needed.
    if (out_type == OUT_ZLIB) {
        vstr_add_strn(&vstr, ZLIB_HEADER, ZLIB_HEADER_LEN);
    } else if (out_type == OUT_GZIP) {
        vstr_add_strn(&vstr, GZIP_HEADER, GZIP_HEADER_LEN);
    }

    // Write starting block, compressed data, and finishing block.
    zlib_start_block(&lz77.outbuf);
    uzlib_lz77_compress(&lz77, bufinfo.buf, bufinfo.len);
    zlib_finish_block(&lz77.outbuf);

    // Write footer if needed.
    if (out_type != OUT_RAW) {
        char footer[8];
        size_t footer_len;
        if (out_type == OUT_ZLIB) {
            uint32_t input_adler32 = uzlib_adler32(bufinfo.buf, bufinfo.len, 1);
            put_be32(&footer[0], input_adler32);
            footer_len = 4;
        } else {
            uint32_t input_crc32 = uzlib_crc32(bufinfo.buf, bufinfo.len, ~0);
            put_le32(&footer[0], ~input_crc32);
            put_le32(&footer[4], bufinfo.len);
            footer_len = 8;
        }
        vstr_add_strn(&vstr, footer, footer_len);
    }

    // Create the resulting bytes object.
    mp_obj_t res = mp_obj_new_bytes_from_vstr(&vstr);

    return res;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_uzlib_compress_obj, 1, 3, mod_uzlib_compress);

#endif // MICROPY_PY_UZLIB_COMPRESS

typedef struct _mp_obj_decompio_t {
    mp_obj_base_t base;
    mp_obj_t src_stream;
    TINF_DATA decomp;
    bool eof;
} mp_obj_decompio_t;

STATIC int read_src_stream(TINF_DATA *data) {
    byte *p = (void *)data;
    p -= offsetof(mp_obj_decompio_t, decomp);
    mp_obj_decompio_t *self = (mp_obj_decompio_t *)p;

    const mp_stream_p_t *stream = mp_get_stream(self->src_stream);
    int err;
    byte c;
    mp_uint_t out_sz = stream->read(self->src_stream, &c, 1, &err);
    if (out_sz == MP_STREAM_ERROR) {
        mp_raise_OSError(err);
    }
    if (out_sz == 0) {
        mp_raise_type(&mp_type_EOFError);
    }
    return c;
}

STATIC mp_obj_t decompio_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 2, false);
    mp_get_stream_raise(args[0], MP_STREAM_OP_READ);
    mp_obj_decompio_t *o = mp_obj_malloc(mp_obj_decompio_t, type);
    memset(&o->decomp, 0, sizeof(o->decomp));
    o->decomp.readSource = read_src_stream;
    o->src_stream = args[0];
    o->eof = false;

    mp_int_t dict_opt = 0;
    uint dict_sz;
    if (n_args > 1) {
        dict_opt = mp_obj_get_int(args[1]);
    }

    if (dict_opt >= 16) {
        int st = uzlib_gzip_parse_header(&o->decomp);
        if (st != TINF_OK) {
            goto header_error;
        }
        dict_sz = 1 << (dict_opt - 16);
    } else if (dict_opt >= 0) {
        dict_opt = uzlib_zlib_parse_header(&o->decomp);
        if (dict_opt < 0) {
        header_error:
            mp_raise_ValueError(MP_ERROR_TEXT("compression header"));
        }
        // RFC 1950 section 2.2:
        // CINFO is the base-2 logarithm of the LZ77 window size,
        // minus eight (CINFO=7 indicates a 32K window size)
        dict_sz = 1 << (dict_opt + 8);
    } else {
        dict_sz = 1 << -dict_opt;
    }

    uzlib_uncompress_init(&o->decomp, m_new(byte, dict_sz), dict_sz);
    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_uint_t decompio_read(mp_obj_t o_in, void *buf, mp_uint_t size, int *errcode) {
    mp_obj_decompio_t *o = MP_OBJ_TO_PTR(o_in);
    if (o->eof) {
        return 0;
    }

    o->decomp.dest = buf;
    o->decomp.dest_limit = (byte *)buf + size;
    int st = uzlib_uncompress_chksum(&o->decomp);
    if (st == TINF_DONE) {
        o->eof = true;
    }
    if (st < 0) {
        DEBUG_printf("uncompress error=" INT_FMT "\n", st);
        *errcode = MP_EINVAL;
        return MP_STREAM_ERROR;
    }
    return o->decomp.dest - (byte *)buf;
}

#if !MICROPY_ENABLE_DYNRUNTIME
STATIC const mp_rom_map_elem_t decompio_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj) },
};

STATIC MP_DEFINE_CONST_DICT(decompio_locals_dict, decompio_locals_dict_table);
#endif

STATIC const mp_stream_p_t decompio_stream_p = {
    .read = decompio_read,
};

#if !MICROPY_ENABLE_DYNRUNTIME
STATIC MP_DEFINE_CONST_OBJ_TYPE(
    decompio_type,
    MP_QSTR_DecompIO,
    MP_TYPE_FLAG_NONE,
    make_new, decompio_make_new,
    protocol, &decompio_stream_p,
    locals_dict, &decompio_locals_dict
    );
#endif

STATIC mp_obj_t mod_uzlib_decompress(size_t n_args, const mp_obj_t *args) {
    mp_obj_t data = args[0];
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);

    TINF_DATA *decomp = m_new_obj(TINF_DATA);
    memset(decomp, 0, sizeof(*decomp));
    DEBUG_printf("sizeof(TINF_DATA)=" UINT_FMT "\n", sizeof(*decomp));
    uzlib_uncompress_init(decomp, NULL, 0);
    mp_uint_t dest_buf_size = (bufinfo.len + 15) & ~15;
    byte *dest_buf = m_new(byte, dest_buf_size);

    decomp->dest = dest_buf;
    decomp->dest_limit = dest_buf + dest_buf_size;
    DEBUG_printf("uzlib: Initial out buffer: " UINT_FMT " bytes\n", dest_buf_size);
    decomp->source = bufinfo.buf;
    decomp->source_limit = (byte *)bufinfo.buf + bufinfo.len;

    int st;
    bool is_zlib = true;

    if (n_args > 1 && MP_OBJ_SMALL_INT_VALUE(args[1]) < 0) {
        is_zlib = false;
    }

    if (is_zlib) {
        st = uzlib_zlib_parse_header(decomp);
        if (st < 0) {
            goto error;
        }
    }

    while (1) {
        st = uzlib_uncompress_chksum(decomp);
        if (st < 0) {
            goto error;
        }
        if (st == TINF_DONE) {
            break;
        }
        size_t offset = decomp->dest - dest_buf;
        dest_buf = m_renew(byte, dest_buf, dest_buf_size, dest_buf_size + 256);
        dest_buf_size += 256;
        decomp->dest = dest_buf + offset;
        decomp->dest_limit = decomp->dest + 256;
    }

    mp_uint_t final_sz = decomp->dest - dest_buf;
    DEBUG_printf("uzlib: Resizing from " UINT_FMT " to final size: " UINT_FMT " bytes\n", dest_buf_size, final_sz);
    dest_buf = (byte *)m_renew(byte, dest_buf, dest_buf_size, final_sz);
    mp_obj_t res = mp_obj_new_bytearray_by_ref(final_sz, dest_buf);
    m_del_obj(TINF_DATA, decomp);
    return res;

error:
    mp_raise_type_arg(&mp_type_ValueError, MP_OBJ_NEW_SMALL_INT(st));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_uzlib_decompress_obj, 1, 3, mod_uzlib_decompress);

#if !MICROPY_ENABLE_DYNRUNTIME
STATIC const mp_rom_map_elem_t mp_module_uzlib_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_uzlib) },
    #if MICROPY_PY_UZLIB_COMPRESS
    { MP_ROM_QSTR(MP_QSTR_compress), MP_ROM_PTR(&mod_uzlib_compress_obj) },
    { MP_ROM_QSTR(MP_QSTR_CompIO), MP_ROM_PTR(&compio_type) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_decompress), MP_ROM_PTR(&mod_uzlib_decompress_obj) },
    { MP_ROM_QSTR(MP_QSTR_DecompIO), MP_ROM_PTR(&decompio_type) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_uzlib_globals, mp_module_uzlib_globals_table);

const mp_obj_module_t mp_module_uzlib = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_uzlib_globals,
};


MP_REGISTER_MODULE(MP_QSTR_uzlib, mp_module_uzlib);
#endif

// Source files #include'd here to make sure they're compiled in
// only if module is enabled by config setting.

#include "lib/uzlib/tinflate.c"
#include "lib/uzlib/tinfzlib.c"
#include "lib/uzlib/tinfgzip.c"
#include "lib/uzlib/adler32.c"
#include "lib/uzlib/crc32.c"

#if MICROPY_PY_UZLIB_COMPRESS
#include "lib/uzlib/defl_static.c"
#include "lib/uzlib/lz77.c"
#endif

#endif // MICROPY_PY_UZLIB
