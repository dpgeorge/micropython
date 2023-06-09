import('../../build/micropython.mjs').then(mp=>{
mp.loadMicroPython({url:"../../build/micropython.wasm"}).then(py=>{
    py.runPython("1");
    py.runPython("print('hello')");
    globalThis.jsadd = function(x, y) { return x + y; };
    py.FS.mkdir("/lib/");
    py.FS.writeFile("/lib/testmod.py", "x = 1; print(__name__, x)");
    py.runPython("import sys; print(f'hello from {sys.platform}')");
    py.runPython("import collections; print(collections.OrderedDict)");
    py.runPython("import js; print(js.jsadd(4, 9))");
    py.runPython("def change_colour():\n print('change_colour')");
    py.runPython("import js; js.setTimeout(change_colour, 100)");
    const testmod = py.pyimport("testmod");
    console.log("testmod:", testmod, testmod.x);
    const importlib = py.pyimport("importlib");
    console.log("importlib:", importlib);
    importlib.invalidate_caches();
    py.registerJsModule("js_module", { y: 2 });
    py.runPython("import js_module; print(js_module); print(js_module.y)");

    py.runPython("obj = js.Object(a=1)");
    console.log("main", py.pyimport("__main__").obj);

    console.log("=======");
    py.runPython(`
from js import Array, Promise, Reflect

def callback(resolve, reject):
  resolve('OK1')

p = Reflect.construct(Promise, Array(callback))
p.then(print)
`);

    console.log("=======");
    py.runPython(`
from js import Promise

def callback(resolve, reject):
  resolve('OK2')

p = Promise.new(callback)
p.then(print)
`);
});

});
