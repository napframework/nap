from typing import Iterable

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *


class PinItem(QGraphicsPathItem):
    """ A Pinitem is a connector without the label """

    def __init__(self, socket):
        """
        @type socket: patch.socketitem.SocketItem
        """
        super(PinItem, self).__init__(socket)
        self._socket = socket

        self.__color = QColor('#000000')

        self.setPen(QPen(Qt.NoPen))
        self.setBrush(self.__color)

        p = QPainterPath()
        p.addRect(0, 0, 10, 10)
        self.setPath(p)

    def socket(self):
        return self._socket


class SocketItem(QGraphicsItem):
    def __init__(self, name):
        super(SocketItem, self).__init__()
        self.__layoutDirty = False
        self._label = QGraphicsTextItem(self)
        self.setName(name)

    def node(self):
        return self.parentItem()

    def isConnected(self):
        return bool(self.edge())

    def edge(self):
        from patch.edgeitem import EdgeItem
        for e in self.scene().edges():
            assert isinstance(e, EdgeItem)
            if e.srcSocket == self: return e
            if e.dstSocket == self: return e
        return None

    def setDirty(self):
        self.__layoutDirty = True

    def setName(self, name: str):
        self._label.setPlainText(name)
        self.setDirty()

    def name(self):
        return self._label.toPlainText()

    def boundingRect(self):
        return self.childrenBoundingRect()

    def paint(self, painter, option, widget=None):
        if self.__layoutDirty:
            self.layout()
            self.__layoutDirty = False

    def setUsable(self, b: bool):
        self.setVisible(b)


class NodeItem(QGraphicsObject):
    def __init__(self):
        super(NodeItem, self).__init__()
        self.setFlag(QGraphicsObject.ItemSendsScenePositionChanges, True)
        self.setFlag(QGraphicsObject.ItemSendsGeometryChanges, True)

    def itemChange(self, change, value):
        if not self.scene():
            return super(NodeItem, self).itemChange(change, value)
        if change == QGraphicsObject.ItemPositionChange:
            self.scene().itemMoved(self)
        return super(NodeItem, self).itemChange(change, value)

    def edges(self):
        """
        :rtype: Iterable[EdgeItem]
        """
        for s in self.sockets():
            e = s.edge()
            if e: yield e

    def sockets(self) -> Iterable[SocketItem]:
        yield
        return
