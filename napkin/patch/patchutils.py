from collections import namedtuple

from PyQt5.QtCore import QPointF
from PyQt5.QtGui import QColor, QPainterPath

import nap
from utils.butils import _clamp

COL_NODE_CONNECTION = QColor(0x70, 0x70, 0x70)
COL_NODE_TITLE = QColor(0xD0, 0xD0, 0xD0)
PATCH_XPOS = '__patch_x_pos__'
PATCH_YPOS = '__patch_y_pos__'


def calculateWirePath(srcPos: QPointF, srcVec: QPointF, dstPos: QPointF, dstVec: QPointF, p: QPainterPath):
    vecsize = 100
    # p.moveTo(srcPos)
    # p.lineTo(dstPos)

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
