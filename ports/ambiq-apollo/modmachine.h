#ifndef MICROPY_INCLUDED_AMBIQ_APOLLO_MODMACHINE_H
#define MICROPY_INCLUDED_AMBIQ_APOLLO_MODMACHINE_H

#include "py/obj.h"

extern const mp_obj_type_t machine_pin_type;

void machine_pin_init(void);
void machine_pin_deinit(void);

#endif // MICROPY_INCLUDED_AMBIQ_APOLLO_MODMACHINE_H
