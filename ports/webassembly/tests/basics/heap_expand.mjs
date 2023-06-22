// Test expanding the MicroPython GC heap.

import {loadMicroPython} from '../../build/micropython.mjs';
const mp = await loadMicroPython({url: '../../build/micropython.wasm'});

mp.runPython(`
import gc
bs = []
for i in range(24):
    b = bytearray(1 << i)
    bs.append(b)
    gc.collect()
    print(gc.mem_free())
for b in bs:
    print(len(b))
`);
