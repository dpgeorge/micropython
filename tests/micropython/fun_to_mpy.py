# Test micropython.fun_to_mpy and micropython.fun_from_mpy.

try:
    from micropython import fun_to_mpy, fun_from_mpy
except ImportError:
    print("SKIP")
    raise SystemExit

import unittest

def fun(a):
    return a + 1

class Test(unittest.TestCase):
    def test_mem8_print(self):
        mpy = fun_to_mpy(fun)
        self.assertIsInstance(mpy, bytes)
        self.assertTrue(mpy.startswith(b"M\x06"))

    def test_alignment(self):
        mpy = fun_to_mpy(fun)
        f2 = fun_from_mpy(mpy)
        self.assertEqual(fun(123), f2(123))


if __name__ == "__main__":
    unittest.main()
