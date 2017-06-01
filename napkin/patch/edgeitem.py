from PyQt5.QtCore import QPointF, Qt
from PyQt5.QtGui import QPen, QPainterPath
from PyQt5.QtWidgets import QGraphicsPathItem, QGraphicsItem

from patch.patchutils import calculateWirePath
from patch.socketitem import SocketItem


class EdgeItemBase(QGraphicsPathItem):
    def __init__(self):
        super(EdgeItemBase, self).__init__()
        self.srcSocket = None
        self.dstSocket = None
        self.srcPos = QPointF()
        self.srcVec = QPointF()
        self.dstPos = QPointF()
        self.dstVec = QPointF()
    
    def update(self):
        self.updatePath()
        super(EdgeItemBase, self).update()
    
    def updatePath(self):
        if self.srcSocket:
            self.srcPos, self.srcVec = self.srcSocket.attachPosVec()
            self.srcPos = self.mapToScene(self.srcPos)
        if self.dstSocket:
            self.dstPos, self.dstVec = self.dstSocket.attachPosVec()
            self.dstPos = self.mapToScene(self.dstPos)

        p = QPainterPath()
        calculateWirePath(self.srcPos, self.srcVec, self.dstPos, self.dstVec, p)
        self.setPath(p)



class EdgeItem(EdgeItemBase):
    def __init__(self, src: SocketItem, dst: SocketItem):
        super(EdgeItem, self).__init__()
        self.srcSocket = src
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
