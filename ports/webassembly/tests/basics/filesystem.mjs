import {loadMicroPython} from '../../build/micropython.mjs';
const mp = await loadMicroPython({url: '../../build/micropython.wasm'});

mp.FS.mkdir("/lib/");
mp.FS.writeFile("/lib/testmod.py", "x = 1; print(__name__, x)");
mp.runPython("import testmod");

mp.runPython("import sys; sys.modules.clear()");
const testmod = mp.pyimport("testmod");
console.log("testmod:", testmod, testmod.x);
