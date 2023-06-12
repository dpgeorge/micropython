import('../../build/micropython.mjs').then(mp=>{
mp.loadMicroPython({url:"../../build/micropython.wasm"}).then(py=>{
    py.registerJsModule("js_module", { y: 2 });
    py.runPython("import js_module; print(js_module); print(js_module.y)");
});
});
