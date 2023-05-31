async function loadMicroPython(options) {
    const {heapsize} = Object.assign({heapsize: 64 * 1024}, options);
    const Module = {};
    Module.locateFile = (path) => "build/" + path;
    const moduleLoaded = new Promise((r) => (Module.postRun = r));
    _createMicroPythonModule(Module);
    await moduleLoaded;
    globalThis.Module = Module;
    proxy_js_init();
    Module.ccall('mp_js_init', 'null', ['number'], [heapsize]);
    Module.ccall('proxy_c_init', 'null', [], []);
    return {
        _module: Module,
        FS : Module.FS,
        globals: {}, // TODO should be a combination of __main__.__dict__ and builtins.__dict__
        registerComlink(comlink) {
        },
        registerJsModule(name, module) {
            const value = Module._malloc(3 * 4);
            convert_js_to_mp_obj_jsside(module, value);
            Module.ccall('mp_js_register_js_module', 'null', ['string', "pointer"], [name, value]);
            Module._free(value);
        },
        pyimport(name) {
            const value = Module._malloc(3 * 4);
            Module.ccall('mp_js_do_import', 'null', ['string', "pointer"], [name, value]);
            return convert_mp_to_js_obj_jsside_with_free(value);
        },
        runPython(code) {
            const value = Module._malloc(3 * 4);
            Module.ccall('mp_js_do_exec', 'number', ['string', "pointer"], [code, value]);
            return convert_mp_to_js_obj_jsside_with_free(value);
        },
    };
}

globalThis.loadMicroPython = loadMicroPython;
