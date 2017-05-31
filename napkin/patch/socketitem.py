from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

class SocketItem(QGraphicsItem):
    def __init__(self, name):
        super(SocketItem, self).__init__()
        self.__layoutDirty = False
        self._label = QGraphicsTextItem(self)
        self._pin = PinItem(self)
        self.setName(name)

    def setDirty(self):
        self.__layoutDirty = True

    def setName(self, name):
        self._label.setPlainText(name)
        self.setDirty()

    def name(self):
        return self._label.toPlainText()

    def boundingRect(self):
        return self.childrenBoundingRect()

    def paint(self, painter, option, widget=None):
        if self.__layoutDirty:
            self._layout()
            self.__layoutDirty = False

    def pin(self):
        return self._pin

        # def attachPos(self):
        #     return self.boundingRect().center()


class InputSocketItem(SocketItem):
    def __init__(self, name):
        super(InputSocketItem, self).__init__(name)
        self.__vec = QPointF(-1, 0)

    def _layout(self):
        pinY = self._label.boundingRect().height() / 2 - self._pin.boundingRect().height() / 2
        self._label.setPos(self.pin().boundingRect().width(), 0)
        self._pin.setPos(0, pinY)
        if self.scene():
            self.scene().update()

    def attachPosVec(self) -> (QPointF, QPointF):
        r = self._pin.boundingRect()
        pos = self._pin.mapToScene(QPointF(r.left(), r.top() + r.height() / 2))
        return pos, self.__vec


class OutputSocketItem(SocketItem):
    def __init__(self, name):
        super(OutputSocketItem, self).__init__(name)
        self.__vec = QPointF(1, 0)

    def _layout(self):
        pinY = self._label.boundingRect().height() / 2 - self._pin.boundingRect().height() / 2
        self._label.setPos(0, 0)
        self._pin.setPos(self._label.boundingRect().width(), pinY)
        if self.scene():
            self.scene().update()

    def attachPosVec(self) -> (QPointF, QPointF):
        r = self._pin.boundingRect()
        pos = self._pin.mapToScene(QPointF(r.right(), r.top() + r.height() / 2))
        return pos, self.__vec


class PinItem(QGraphicsPathItem):
    """ A Pinitem is a connector without the label """

    def __init__(self, plugItem):
        """
        @type plugItem: patch.socketitem.SocketItem
        """
        super(PinItem, self).__init__(plugItem)
        self._PlugItem = plugItem

        self.__color = QColor('#FF00FF')

        self.setPen(QPen(Qt.NoPen))
        self.setBrush(self.__color)

        p = QPainterPath()
        p.addRect(0, 0, 10, 10)
        self.setPath(p)


def inputOutputConnectCondition(src: SocketItem, dst: SocketItem):
    if isinstance(src, InputSocketItem) and isinstance(dst, InputSocketItem):
        return False
    if isinstance(src, OutputSocketItem) and isinstance(src, OutputSocketItem):
        return False
    return True
