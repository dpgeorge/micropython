import('../../build/micropython.mjs').then(mp=>{
mp.loadMicroPython({url:"../../build/micropython.wasm"}).then(py=>{
    globalThis.jsadd = function(x, y) { return x + y; };
    py.runPython("import js; print(js); print(js.jsadd(4, 9))");

    py.runPython("def change_colour():\n print('change_colour')");
    py.runPython("import js; js.setTimeout(change_colour, 100)");

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
