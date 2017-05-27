from PyQt5.QtCore import Qt, QPointF
from PyQt5.QtGui import QPen, QPainterPath
from PyQt5.QtWidgets import QGraphicsPathItem


class PinItem(QGraphicsPathItem):
    """ A Pinitem is a connector without the label """

    def __init__(self, plugItem):
        """
        @type plugItem: patch.plugitem.PlugItem
        """
        super(PinItem, self).__init__(plugItem)
        self.__plugItem = plugItem

        self.__color = self.plugItem().plug().core().typeColor(self.plugItem().plug().dataType())

        self.setPen(QPen(Qt.NoPen))
        self.setBrush(self.__color)

        p = QPainterPath()
        p.addRect(0, 0, 10, 10)
        self.setPath(p)

    def attachPos(self):
        r = self.boundingRect()
        if self.__plugItem.isInput():
            return QPointF(self.scenePos().x() - r.left(),
                           self.scenePos().y() + r.height() / 2)
        return QPointF(self.scenePos().x() + r.right(),
                       self.scenePos().y() + r.height() / 2)

    def plugItem(self):
        return self.__plugItem

    def color(self):
        return self.__color