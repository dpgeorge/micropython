import('../../build/micropython.mjs').then(mp=>{
mp.loadMicroPython({url:"../../build/micropython.wasm"}).then(py=>{
    py.runPython("1");
    py.runPython("print('hello')");
    py.runPython("import sys; print(f'hello from {sys.platform}')");
    py.runPython("import collections; print(collections.OrderedDict)");
});
});
