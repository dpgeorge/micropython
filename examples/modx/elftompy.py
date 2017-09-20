"""
This tool is ultimately intended to convert a .elf to a .mpy file.
But for now all it does is patch the text size within a .mpy file.

Usage (after building modx.mpy):

    $ python elftompy.py modx.mpy

This will make modx.mpy ready for importing.
"""

import sys
import struct

def to_uint(n):
    if n < 0:
        raise ValueError('unsigned integer cannot be < 0')
    b = bytearray()
    while n:
        # set high bit: more bytes will follow
        b.insert(0, (n & 0x7f) | 0x80)
        n >>= 7
    if len(b) == 0:
        return b'\x00'
    # clear high bit: it's the last byte
    b[-1] &= 0x7f
    return bytes(b)


def process_file(filename):
    with open(filename, 'rb') as f:
        header = f.read(4) # .mpy header + number of QSTRs
        num_qstrs = struct.unpack('<H', f.read(2))[0]
        padding = f.read(2) # skip
        text = f.read()

    with open(filename, 'wb') as f:
        f.write(header)
        f.write(to_uint(num_qstrs))
        f.write(to_uint(len(text)))
        f.write(text)

if __name__ == '__main__':
    process_file(sys.argv[1])
