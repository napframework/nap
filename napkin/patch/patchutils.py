from PyQt5.QtCore import QPointF
from PyQt5.QtGui import QColor

import nap
from utils.butils import _clamp


COL_NODE_CONNECTION = QColor(0x70, 0x70, 0x70)
COL_NODE_TITLE = QColor(0xD0, 0xD0, 0xD0)
PATCH_XPOS = '__patch_x_pos__'
PATCH_YPOS = '__patch_y_pos__'


def calculateWirePath(srcPos, dstPos, p):
    """
    @type srcPos: QPointF
    @type dstPos: QPointF
    @type p: QPainterPath
    """
    p.moveTo(srcPos)
    if dstPos.x() > srcPos.x():
        hx = srcPos.x() + (dstPos.x() - srcPos.x()) / 2.0
        c1 = QPointF(hx, srcPos.y())
        c2 = QPointF(hx, dstPos.y())
        p.cubicTo(c1, c2, dstPos)
    else:
        maxDist = 150

        dist = _clamp(srcPos.x() - dstPos.x(), -maxDist, maxDist)

        c1 = QPointF(srcPos.x() + dist, srcPos.y())
        c2 = QPointF(dstPos.x() - dist, dstPos.y())
        p.cubicTo(c1, c2, dstPos)


def _getObjectEditorPos(obj):
    x = obj.attr(PATCH_XPOS).value()
    y = obj.attr(PATCH_YPOS).value()
    return QPointF(x, y)


def _setObjectEditorPos(obj, pos):
    """
    @type obj: nap.AttributeObject
    """
    obj.attr(PATCH_XPOS).setValue(pos.x())
    obj.attr(PATCH_YPOS).setValue(pos.y())


def _canConnect(srcPlug, destPlug):
    srcIsInput = isinstance(srcPlug, nap.InputPlugBase)
    destIsInput = isinstance(destPlug, nap.InputPlugBase)
    if srcIsInput == destIsInput:
        return False
    if srcPlug.parent() == destPlug.parent():
        return False
    return srcPlug.dataType() == destPlug.dataType()


def moveToFront(item):
    highest = -10000000000
    for item in item.scene().items():
        highest = max(highest, item.zValue())
    item.setZValue(highest + 1)