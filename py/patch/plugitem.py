from PyQt5.QtWidgets import QGraphicsItem, QGraphicsTextItem

import nap
from patch.pinitem import PinItem


class PlugItem(QGraphicsItem):
    def __init__(self, opItem, plug):
        super(PlugItem, self).__init__()
        self.__plug = plug
        self.__opItem = opItem
        self.__label = QGraphicsTextItem(self)
        self.__pin = PinItem(self)
        self.setParentItem(self.__opItem)
        self.setName(plug.name())

    def setName(self, name):
        self.__label.setPlainText(name)
        self.layout()

    def boundingRect(self):
        return self.childrenBoundingRect()

    def paint(self, painter, option, widget=None):
        pass

    def pin(self):
        return self.__pin

    def plug(self):
        """
        @rtype: nap.Plug
        """
        return self.__plug

    def isInput(self):
        return isinstance(self.__plug, nap.InputPlugBase)

    def setUsable(self, b):
        self.setVisible(b)

    def layout(self):
        # textOffset = 6

        pinY = self.__label.boundingRect().height() / 2 - self.__pin.boundingRect().height() / 2

        if self.isInput():
            self.__label.setPos(self.pin().boundingRect().width(), 0)
            self.__pin.setPos(0, pinY)
        else:
            self.__label.setPos(0, 0)
            self.__pin.setPos(self.__label.boundingRect().width(), pinY)

        self.__opItem.layout()