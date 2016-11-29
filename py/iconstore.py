"""
This module provides unified access to this projects icons.
"""
from collections import OrderedDict

from PyQt5.QtGui import *
from PyQt5.QtCore import *

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
    """ Retrieve an icon by name as such: iconDir/NAME.png,
    where NAME is the provided string. """
    if not name in _ICONS:
        _ICONS[name] = QIcon('icons/%s.png' % name)
    return _ICONS[name]


def iconFor(obj):
    """ Based on the type of @obj, return an icon.
    """
    if isinstance(obj, type):
        objType = obj
    else:
        objType = type(obj)

    for regType in _ICON_NAMES.keys():
        if issubclass(objType, regType):
            return icon(_ICON_NAMES[regType])
    raise Exception('Failed to get icon')
