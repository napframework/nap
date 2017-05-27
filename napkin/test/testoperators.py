import inspect
import json

import sys

# class operator(object):
#     def __init__(self, **kwargs):
#         super(operator, self).__init__()
#         self._args = kwargs
#
#     def __call__(self, f):
#         def wrap(*args, **kwargs):
#             f(*args, **kwargs)
#         return wrap


# @operator(name='Add')
import math

import collections


def addoperator(*term: float) -> float:
    return sum(term)


def sine(a: float) -> float:
    return math.sin(a())


def printOperatorMeta(f):
    spec = inspect.getfullargspec(f)
    print(spec)
    d = {
        'name': f.__name__,
        'inlets': {k: v.__name__ for k, v in spec.annotations.items() if k != 'return'},
        'outlets': {k: v.__name__ for k, v in spec.annotations.items() if k == 'return'},
    }
    j = json.dumps(d, indent=4, sort_keys=True)
    print(j)


if __name__ == '__main__':
    printOperatorMeta(addoperator)
    printOperatorMeta(sine)

