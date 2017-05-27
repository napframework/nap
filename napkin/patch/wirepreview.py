from PyQt5.QtCore import QPointF, Qt
from PyQt5.QtGui import QPainterPath, QPen
from PyQt5.QtWidgets import QGraphicsPathItem

from patch.patchutils import calculateWirePath, COL_NODE_CONNECTION


class WirePreview(QGraphicsPathItem):
    def __init__(self, parent):
        super(WirePreview, self).__init__(parent)
        self.srcPos = QPointF()
        self.dstPos = QPointF()
        self.srcPin = None
        self.dstPin = None
        self.setAcceptTouchEvents(False)
        self.setAcceptHoverEvents(False)

    def boundingRect(self):
        return self.path().boundingRect()

    def paint(self, painter, option, widget=None):
        if self.isVisible():
            self.updatePath()
            painter.setPen(self.pen())
            painter.drawPath(self.path())

    def updatePath(self):
        if self.srcPin:
            self.srcPos = self.srcPin.attachPos()
        if self.dstPin:
            self.dstPos = self.dstPin.attachPos()

        p = QPainterPath()
        calculateWirePath(self.srcPos, self.dstPos, p)
        self.setPath(p)

        pen = QPen()
        pen.setColor(COL_NODE_CONNECTION)
        pen.setWidth(1)
        pen.setStyle(Qt.DashLine)
        self.setPen(pen)