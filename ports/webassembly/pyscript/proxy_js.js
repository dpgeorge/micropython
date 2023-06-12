class PythonError extends Error {
}
Object.defineProperty(PythonError.prototype, "name", { value: "PythonError" });

function proxy_js_init() {
    globalThis.proxy_js_ref = [globalThis];
}

function proxy_call_python(target, argumentsList) {
    //console.debug("APPLY", target, argumentsList);
    let args = 0;

    // TODO: is this the correct thing to do, strip trailing "undefined" args?
    while (argumentsList.length > 0 && argumentsList[argumentsList.length - 1] === undefined) {
        argumentsList.pop();
    }

    if (argumentsList.length > 0) {
        // TODO use stackAlloc/stackRestore?
        args = Module._malloc(argumentsList.length * 3 * 4);
        for (let i in argumentsList) {
            convert_js_to_mp_obj_jsside(argumentsList[i], args + i * 3 * 4);
        }
    }
    const value = Module._malloc(3 * 4);
    Module.ccall("proxy_c_to_js_call", "null", ["number", "number", "number", "pointer"], [target, argumentsList.length, args, value]);
    if (argumentsList.length > 0) {
        Module._free(args);
    }
    return convert_mp_to_js_obj_jsside_with_free(value);
}

const proxy_handler = {
    isExtensible() {
        return true;
    },
    has(target, prop) {
        throw Error("has not implemented");
    },
    get(target, prop) {
        //console.debug("GET", target._ref, prop);
        if (prop in target || typeof prop === "symbol") {
            return Reflect.get(target, prop);
        }
        const value = Module._malloc(3 * 4);
        Module.ccall("proxy_c_to_js_lookup_attr", "number", ["number", "string", "pointer"], [target._ref, prop, value]);
        return convert_mp_to_js_obj_jsside_with_free(value);
    },
    set(target, prop, value) {
        //console.debug("SET", target, prop, value);
        if (typeof prop === "symbol") {
            return Reflect.set(target, prop, value);
        }
        throw Error("set not implemented");
        return true;
    },
    deleteProperty(target, prop) {
        throw Error("deleteProperty not implemented");
    },
    ownKeys(target) {
        throw Error("ownKeys not implemented");
    },
};

function convert_js_to_mp_obj_jsside(js_obj, out) {
    let kind;
    if (js_obj === null) {
        kind = 1;
    } else if (typeof js_obj === "boolean") {
        kind = 2;
        Module.setValue(out + 4, js_obj, "i32");
    } else if (typeof js_obj === "number") {
        kind = 3;
        Module.setValue(out + 4, js_obj, "i32");
    } else if (typeof js_obj === "string") {
        kind = 4;
        const len = Module.lengthBytesUTF8(js_obj);
        const buf = Module._malloc(len + 1);
        Module.stringToUTF8(js_obj, buf, len + 1);
        Module.setValue(out + 4, len, "i32");
        Module.setValue(out + 8, buf, "i32");
    } else if (typeof js_obj === "object" && "_ref" in js_obj) {
        // Is this our Proxy object??
        throw new Error("convert_js_to_mp_obj_jsside: not implemented");
    } else {
        kind = 5;
        const id = proxy_js_ref.length;
        proxy_js_ref[id] = js_obj;
        Module.setValue(out + 4, id, "i32");
    }
    Module.setValue(out + 0, kind, "i32");
    //console.debug("convert_js_to_mp_obj_jsside", js_obj, out, "->", kind, Module.getValue(out + 4, "i32"));
}

function convert_mp_to_js_obj_jsside(value) {
    const kind = Module.getValue(value, "i32");
    let obj;
    if (kind == -1) {
        // Exception
        const str_len = Module.getValue(value + 4, "i32");
        const str_ptr = Module.getValue(value + 8, "i32");
        const str = Module.UTF8ToString(str_ptr, str_len);
        Module._free(str_ptr);
        console.error("MicroPython exception:", str);
        throw new PythonError(str);
    }
    if (kind == 0) {
        // MP_OBJ_NULL
        throw new Error("NULL object");
    } else if (kind == 1) {
        // None
        obj = null;
    } else if (kind == 2) {
        // bool
        obj = Module.getValue(value + 4, "i32") ? true : false;
    } else if (kind == 3) {
        // int
        obj = Module.getValue(value + 4, "i32");
    } else if (kind == 4) {
        // str
        const str_len = Module.getValue(value + 4, "i32");
        const str_ptr = Module.getValue(value + 8, "i32");
        obj = Module.UTF8ToString(str_ptr, str_len);
    } else if (kind == 7) {
        // js obj
        const id = Module.getValue(value + 4, "i32");
        obj = proxy_js_ref[id];
    } else {
        // obj
        const id = Module.getValue(value + 4, "i32");
        if (kind == 5) {
            obj = function(...args) {
                return proxy_call_python(id, args);
            };
        } else {
            let target = {};
            target._ref = id;
            obj = new Proxy(target, proxy_handler);
        }
    }
    //console.debug("convert_mp_to_js_obj_jsside", value, kind, "->", obj, kind == 5 ? Module.getValue(value + 4, "i32") : "");
    return obj;
}

function convert_mp_to_js_obj_jsside_with_free(value) {
    const ret = convert_mp_to_js_obj_jsside(value);
    Module._free(value);
    return ret;
}

function python_index_semantics(target, index) {
    if (typeof index === "number") {
        if (index < 0) {
            index += target.length;
        }
        if (index < 0 || index >= target.length) {
            throw new PythonError("index out of range");
        }
    }
    return index;
}
