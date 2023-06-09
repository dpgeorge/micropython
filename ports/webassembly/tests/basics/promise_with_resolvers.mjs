// Test polyfill of a method on a built-in.

Promise.withResolvers || (Promise.withResolvers = function withResolvers() {
  var a, b, c = new this(function (resolve, reject) {
    a = resolve;
    b = reject;
  });
  return {resolve: a, reject: b, promise: c};
});

import {loadMicroPython} from '../../build/micropython.mjs';
const runtime = await loadMicroPython({url: '../../build/micropython.wasm'});

// why is this needed? comment this and see the error
globalThis.Promise = class extends Promise {};

runtime.runPython(`
    from js import Promise

    deferred = Promise.withResolvers()
    deferred.promise.then(print)
    deferred.resolve('OK')
`);
