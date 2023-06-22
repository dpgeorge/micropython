// Test accessing Python globals dict via mp.globals.

import {loadMicroPython} from '../../build/micropython.mjs';
const mp = await loadMicroPython({url: '../../build/micropython.wasm'});

mp.runPython("x = 1");
console.log(mp.globals.get("x"));

mp.globals.set("y", 2);
mp.runPython("print(y)");

mp.runPython("print('y' in globals())");
mp.globals.delete("y");
mp.runPython("print('y' in globals())");
