import sys

import os
import nap

def napClasses(baseClass=None):
    for k, v in nap.__dict__.items():
        if not isinstance(v, type): continue
        if baseClass and not issubclass(v, baseClass): continue
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