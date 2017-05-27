"""
This module provides unified access to this project's icons.
"""
from collections import OrderedDict

from PyQt5.QtGui import *

import nap

# Store icons as type:QIcon
_ICONS = {}

_ICON_NAMES = OrderedDict([
    (nap.Entity, 'entity'),
    (nap.Attribute, 'attribute'),
    (nap.Component, 'component'),
    (nap.Object, 'object'),
])


def icon(name):
    """ Retrieve an icon filename by name as such: iconDir/NAME.png,
    where NAME is the provided string.

    icon('computer/disk') # returns 'path/to/computer/disk.png'
    """
    if not name in _ICONS:
        _ICONS[name] = QIcon('icons/%s.png' % name)
    return _ICONS[name]


def iconFor(obj):
    """ Based on the type of @obj, return an icon filename.
    """
    if isinstance(obj, type):
        objType = obj
    else:
        objType = type(obj)

    for regType in _ICON_NAMES.keys():
        if issubclass(objType, regType):
            return icon(_ICON_NAMES[regType])
    raise Exception('Failed to get icon')
