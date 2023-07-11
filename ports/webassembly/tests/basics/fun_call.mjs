// Test calling JavaScript functions from Python.

import {loadMicroPython} from '../../build/micropython.mjs';
const mp = await loadMicroPython({url: '../../build/micropython.wasm'});

globalThis.f = function(a, b, c, d, e) { console.log(a, b, c, d, e); };
mp.runPython(`
import js
js.f()
js.f(1)
js.f(1, 2)
js.f(1, 2, 3)
js.f(1, 2, 3, 4)
js.f(1, 2, 3, 4, 5)
js.f(1, 2, 3, 4, 5, 6)
`);
