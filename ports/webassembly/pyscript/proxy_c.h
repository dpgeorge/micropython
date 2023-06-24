#include "py/obj.h"

void proxy_c_init(void);
mp_obj_t convert_js_to_mp_obj_cside(uint32_t *value);
void convert_mp_to_js_obj_cside(mp_obj_t obj, uint32_t *out);
void convert_mp_to_js_exc_cside(void *exc, uint32_t *out);

mp_obj_t mp_obj_new_jsobj(int ref);

bool mp_obj_is_jsobj(mp_obj_t o);
int mp_obj_jsobj_get_ref(mp_obj_t o);

MP_DECLARE_CONST_FUN_OBJ_3(then_obj);
