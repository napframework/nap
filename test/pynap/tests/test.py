import sys

import os

sys.path.append(r'%s/../build/lib.linux-x86_64-3.5' %
                os.path.dirname(__file__))

import pynap as m

assert m.__version__ == '0.0.1'
assert m.add(1, 2) == 3
assert m.subtract(1, 2) == -1

print('Tests OK')
