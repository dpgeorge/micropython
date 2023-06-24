/*
import { createRequire } from 'module';
const require = createRequire(import.meta.url);
var __dirname = "";
*/

export async function loadMicroPython(options) {
    const {heapsize, url, stdin, stdout, stderr} = Object.assign({heapsize: 1024 * 1024}, options);
    const Module = {};
    Module.locateFile = (path, scriptDirectory) => url || path;
    Module._textDecoder = new TextDecoder();
    if (stdin !== undefined) {
        Module.stdin = stdin;
    }
    if (stdout !== undefined) {
        Module._stdoutBuffer = [];
        Module.stdout = function(c) {
            if (c == 10) {
                stdout(Module._textDecoder.decode(new Uint8Array(Module._stdoutBuffer)));
                Module._stdoutBuffer = [];
            } else {
                Module._stdoutBuffer.push(c);
            }
        };
    }
    if (stderr !== undefined) {
        Module._stderrBuffer = [];
        Module.stderr = function(c) {
            if (c == 10) {
                stderr(Module._textDecoder.decode(new Uint8Array(Module._stderrBuffer)));
                Module._stderrBuffer = [];
            } else {
                Module._stderrBuffer.push(c);
            }
        };
    }
    const moduleLoaded = new Promise((r) => (Module.postRun = r));
    _createMicroPythonModule(Module);
    await moduleLoaded;
    globalThis.Module = Module;
    proxy_js_init();
    const pyimport = function(name) {
        const value = Module._malloc(3 * 4);
        Module.ccall('mp_js_do_import', 'null', ['string', "pointer"], [name, value]);
        return convert_mp_to_js_obj_jsside_with_free(value);
    };
    Module.ccall('mp_js_init', 'null', ['number'], [heapsize]);
    Module.ccall('proxy_c_init', 'null', [], []);
    return {
        _module: Module,
        FS : Module.FS,
        globals: pyimport("__main__").__dict__,
        registerJsModule(name, module) {
            const value = Module._malloc(3 * 4);
            convert_js_to_mp_obj_jsside(module, value);
            Module.ccall('mp_js_register_js_module', 'null', ['string', "pointer"], [name, value]);
            Module._free(value);
        },
        pyimport: pyimport,
        runPython(code) {
            const value = Module._malloc(3 * 4);
            Module.ccall('mp_js_do_exec', 'number', ['string', "pointer"], [code, value]);
            return convert_mp_to_js_obj_jsside_with_free(value);
        },
        runPythonAsync(code) {
            const value = Module._malloc(3 * 4);
            Module.ccall('mp_js_do_exec_async', 'number', ['string', "pointer"], [code, value]);
            return convert_mp_to_js_obj_jsside_with_free(value);
        },
    };
}

globalThis.loadMicroPython = loadMicroPython;
