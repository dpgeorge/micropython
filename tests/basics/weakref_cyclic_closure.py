# CPython's `weakref.finalize` is documented to leak memory in certain cases,
# per https://docs.python.org/3/library/weakref.html#weakref.finalize.
# This is not fundamental, though --- and MicroPython actually does NOT leak here.

try:
    import weakref
except ImportError:
    print("SKIP")
    raise SystemExit

import gc

item_count = 1024  # number of leaky finalizers to create
item_size = 1024  # size of each leaky finalizer's object

item_fin_lb = item_count // 2
mem_leak_ub = item_count // 2 * item_size

item_fin = 0
def fin(buf):
    global item_fin
    item_fin += 1

def leak(sz):
    buf = bytearray(sz)
    weakref.finalize(buf, fin, buf)

gc.collect()
mem_before = gc.mem_alloc()

for i in range(item_count):
    leak(item_size)
    gc.collect()

gc.collect()
mem_after = gc.mem_alloc()

mem_leak = mem_after - mem_before

if item_fin > item_fin_lb:
    print("collected >", item_fin_lb)
else:
    print("collected =", item_fin)

if mem_leak < mem_leak_ub:
    print("leaked <", mem_leak_ub)
else:
    print("leaked =", mem_leak)
