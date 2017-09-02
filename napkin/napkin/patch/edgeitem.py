import math
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from napkin.patch.inoutnodeitem import InputSocketItem
from napkin.patch.inoutnodeitem import OutputSocketItem
from napkin.patch.nodeitem import SocketItem
from napkin.patch.patchutils import calculateWirePath


class EdgeItemBase(QGraphicsPathItem):
    def __init__(self):
        super(EdgeItemBase, self).__init__()
        # type: OutputSocketItem
        self.srcSocket = None
        # type: InputSocketItem
        self.dstSocket = None
        self.srcPos = QPointF()
        self.srcVec = QPointF()
        self.dstPos = QPointF()
        self.dstVec = QPointF()
        self.defaultCol = QColor("#888888")

        self.brush = QLinearGradient()
        self.brush.setCoordinateMode(QLinearGradient.ObjectBoundingMode)
        self.brush.setColorAt(0, QColor('#FF0000'))
        self.brush.setColorAt(1, QColor('#00FF00'))
        self.brush.setStart(0, 0)
        self.brush.setFinalStop(1, 1)

    def update(self, *args):
        self.updatePath()
        super(EdgeItemBase, self).update()

    def updatePath(self):
        if self.srcSocket:
            self.srcPos, self.srcVec = self.srcSocket.attachPosVec()
            self.srcPos = self.mapToScene(self.srcPos)
            self.brush.setColorAt(0, self.srcSocket.pinColor())
        else:
            self.brush.setColorAt(0, self.defaultCol)

        if self.dstSocket:
            self.dstPos, self.dstVec = self.dstSocket.attachPosVec()
            self.dstPos = self.mapToScene(self.dstPos)
            self.brush.setColorAt(1, self.dstSocket.pinColor())
        else:
            self.brush.setColorAt(1, self.defaultCol)

        sx = self.srcPos.x()
        sy = self.srcPos.y()
        tx = self.dstPos.x()
        ty = self.dstPos.y()
        a = math.atan2(ty - sy, tx - sx)

        if a < -math.pi/2:
            self.brush.setStart(0.5, 1)
            self.brush.setFinalStop(0.5, 0)
        elif a > math.pi/2:
            self.brush.setStart(0.5, 0)
            self.brush.setFinalStop(0.5, 1)
        elif sy > ty:
            self.brush.setStart(0, 1)
            self.brush.setFinalStop(0, 0)
        else:
            self.brush.setStart(0, 0)
            self.brush.setFinalStop(0, 1)


        p = QPainterPath()
        calculateWirePath(self.srcPos, self.srcVec, self.dstPos, self.dstVec, p)
        self.setPath(p)

    def paint(self, painter: QPainter, option: QStyleOptionGraphicsItem, widget: QWidget = None):
        pen = painter.pen()
        pen.setBrush(self.brush)
        if self.isSelected():
            pen.setWidth(3)
        painter.setPen(pen)
        painter.drawPath(self.path())


class EdgeItem(EdgeItemBase):
    def __init__(self, src: SocketItem, dst: SocketItem):
        super(EdgeItem, self).__init__()
        # type:SocketItem
        self.srcSocket = src
        # type:SocketItem
        self.dstSocket = dst
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)
        self.updatePath()


class PreviewEdge(EdgeItemBase):
    def __init__(self):
        super(PreviewEdge, self).__init__()
        self.setAcceptTouchEvents(False)
        self.setAcceptHoverEvents(False)

        pen = QPen()
        pen.setWidth(1)
        pen.setStyle(Qt.DashLine)
        self.setPen(pen)

    def boundingRect(self):
        return self.path().boundingRect()
