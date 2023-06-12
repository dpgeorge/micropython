// Test overriding .new() on a JavaScript class.

import {loadMicroPython} from '../../build/micropython.mjs';
const runtime = await loadMicroPython({url: '../../build/micropython.wasm'});

globalThis.MyClass = class {
    new() {
        console.log("MyClass.new");
    }
};

globalThis.myClassInstance = new globalThis.MyClass();

runtime.runPython(`
    import js
    js.MyClass.new().new()
    js.myClassInstance.new()
`);
