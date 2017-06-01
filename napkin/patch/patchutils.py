from collections import namedtuple

import math
from PyQt5.QtCore import *
from PyQt5.QtGui import *

Margins = namedtuple('Margins', ['top', 'right', 'bottom', 'left'])

COL_NODE_CONNECTION = QColor(0x70, 0x70, 0x70)
COL_NODE_TITLE = QColor(0xD0, 0xD0, 0xD0)
PATCH_XPOS = '__patch_x_pos__'
PATCH_YPOS = '__patch_y_pos__'


def _lerp(p1: QPointF, p2: QPointF, t: float) -> QPointF:
    return p1 * (1 - t) + p2 * t


def _dist(p1: QPointF, p2: QPointF) -> float:
    d = p2 - p1
    return math.sqrt(d.x() * d.x() + d.y() * d.y())


def calculateWirePath(srcPos: QPointF, srcVec: QPointF, dstPos: QPointF, dstVec: QPointF,
                      p: QPainterPath):

    dist = _dist(srcPos, dstPos)
    # p.moveTo(srcPos)
    # p.lineTo(dstPos)
    vecsize = dist/2

    # return
    c1 = srcPos + srcVec * vecsize
    c2 = dstPos + dstVec * vecsize

    p.moveTo(srcPos)
    p.cubicTo(c1, c2, dstPos)


def _getObjectEditorPos(obj):
    x = obj.attr(PATCH_XPOS).value()
    y = obj.attr(PATCH_YPOS).value()
    return QPointF(x, y)


def moveToFront(item):
    highest = -10000000000
    for item in item.scene().items():
        highest = max(highest, item.zValue())
    item.setZValue(highest + 1)
