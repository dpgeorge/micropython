include ../../py/mkenv.mk

CROSS = 0

QSTR_DEFS = qstrdefsport.h

include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

CC = emcc
LD = emcc

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

CFLAGS += -std=c99 -Wall -Werror -Wdouble-promotion -Wfloat-conversion
CFLAGS += -Os -DNDEBUG
CFLAGS += $(INC)

SRC_SHARED = $(addprefix shared/,\
	runtime/interrupt_char.c \
	runtime/stdout_helpers.c \
	runtime/pyexec.c \
	readline/readline.c \
	)

SRC_C = \
	main.c \
	mphalport.c \

SRC_QSTR += $(SRC_C) $(SRC_SHARED)

SRC_JS ?= wrapper.js

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_SHARED:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

JSFLAGS += -s ASYNCIFY
JSFLAGS += -s EXPORTED_FUNCTIONS="\
	_mp_js_init,\
	_mp_js_init_repl,\
	_mp_js_do_str,\
	_mp_js_process_char,\
	_mp_hal_get_interrupt_char,\
	_mp_sched_keyboard_interrupt$(EXPORTED_FUNCTIONS_EXTRA)"
JSFLAGS += -s EXPORTED_RUNTIME_METHODS="\
	ccall,\
	cwrap,\
	FS$(EXPORTED_RUNTIME_METHODS_EXTRA)"
JSFLAGS += -s --memory-init-file 0
JSFLAGS += --js-library library.js

all: $(BUILD)/micropython.js

$(BUILD)/micropython.js: $(OBJ) library.js $(SRC_JS)
	$(ECHO) "LINK $@"
	$(Q)emcc $(LDFLAGS) -o $@ $(OBJ) $(JSFLAGS)
	$(Q)cat $(SRC_JS) >> $@

min: $(BUILD)/micropython.js
	uglifyjs $< -c -o $(BUILD)/micropython.min.js

test: $(BUILD)/micropython.js $(TOP)/tests/run-tests.py
	$(eval DIRNAME=ports/$(notdir $(CURDIR)))
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../ports/webassembly/node_run.sh ./run-tests.py -j1

include $(TOP)/py/mkrules.mk
