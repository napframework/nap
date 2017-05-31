from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *


class PinItem(QGraphicsPathItem):
    """ A Pinitem is a connector without the label """

    def __init__(self, plugItem):
        """
        @type plugItem: patch._plugitem._PlugItem
        """
        super(PinItem, self).__init__(plugItem)
        self._PlugItem = plugItem

        self.__color = QColor('#FF00FF')

        self.setPen(QPen(Qt.NoPen))
        self.setBrush(self.__color)

        p = QPainterPath()
        p.addRect(0, 0, 10, 10)
        self.setPath(p)

    def attachPos(self):
        r = self.boundingRect()
        if self._PlugItem.isInput():
            return QPointF(self.scenePos().x() - r.left(),
                           self.scenePos().y() + r.height() / 2)
        return QPointF(self.scenePos().x() + r.right(),
                       self.scenePos().y() + r.height() / 2)

    def plugItem(self):
        return self._PlugItem

    def color(self):
        return self.__color