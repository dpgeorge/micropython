SRC_C += \
	pyscript/extra.c \
	pyscript/modjs.c \
	pyscript/modpyodide.c \
	pyscript/proxy_c.c \

SRC_JS = pyscript/extra.js pyscript/proxy_js.js

EXPORTED_FUNCTIONS_EXTRA += ,\
	_malloc,\
	_free,\
	_mp_js_do_exec,\
	_mp_js_do_exec_async,\
	_mp_js_do_import,\
	_mp_js_register_js_module,\
	_mp_obj_jsobj_callback_proxy,\
	_proxy_c_init,\
	_proxy_c_to_js_call,\
	_proxy_c_to_js_lookup_attr

EXPORTED_RUNTIME_METHODS_EXTRA += ,\
	getValue,\
	setValue,\
	lengthBytesUTF8,\
	stringToUTF8,\
	UTF8ToString,\
	PATH,\
	PATH_FS

JSFLAGS += -s MODULARIZE -s EXPORT_NAME=_createMicroPythonModule
JSFLAGS += -s ALLOW_MEMORY_GROWTH

FROZEN_MANIFEST ?= pyscript/manifest.py
