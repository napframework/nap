import sys

import os

# TODO: Dirty hack to get things going right now
p = '%s/../../lib/Clang-Debug-x86_64' % os.path.dirname(__file__)
sys.path.append(p)
import nap

def napClasses():
    for k, v in nap.__dict__.items():
        if not isinstance(v, type): continue
        yield v


def baseClassList(cls, outlist=None):
    if not outlist:
        outlist = []
    base = next((c for c in cls.__bases__), None)
    if not base:
        return outlist
    outlist.append(base)
    baseClassList(base, outlist)
    return outlist