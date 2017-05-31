from PyQt5.QtCore import QPointF
from PyQt5.QtGui import QPen, QPainterPath
from PyQt5.QtWidgets import QGraphicsPathItem, QGraphicsItem

from patch.patchutils import calculateWirePath


class EdgeItem(QGraphicsPathItem):
    def __init__(self, srcPinItem, dstPinItem):
        """
        @type srcPinItem: patch.pinitem.PinItem
        @type dstPinItem: patch.pinitem.PinItem
        """
        super(EdgeItem, self).__init__()
        self.srcPin = srcPinItem
        self.dstPin = dstPinItem
        self.srcPos = QPointF()
        self.dstPos = QPointF()
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)
        self.updatePath()

    def paint(self, painter, option, widget=None):
        if self.isVisible():
            self.updatePath()
        super(EdgeItem, self).paint(painter, option, widget)

    def updatePath(self):
        if self.srcPin:
            self.srcPos = self.srcPin.attachPos()
        if self.dstPin:
            self.dstPos = self.dstPin.attachPos()

        pen = QPen()
        pen.setColor(self.srcPin.color())
        pen.setWidth(1)
        self.setPen(pen)

        p = QPainterPath()
        calculateWirePath(self.srcPos, self.dstPos, p)
        self.setPath(p)