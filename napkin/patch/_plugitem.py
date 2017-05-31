from PyQt5.QtWidgets import QGraphicsItem, QGraphicsTextItem

import nap
from patch.pinitem import PinItem



class _PlugItem(QGraphicsItem):
    def __init__(self, name):
        super(_PlugItem, self).__init__()
        self._label = QGraphicsTextItem(self)
        self._pin = PinItem(self)
        self.setName(name)

    def setName(self, name):
        self._label.setPlainText(name)

    def boundingRect(self):
        return self.childrenBoundingRect()

    def paint(self, painter, option, widget=None):
        self._layout()

    def pin(self):
        return self._pin


class InPlugItem(_PlugItem):
    def __init__(self, name):
        super(InPlugItem, self).__init__(name)

    def _layout(self):
        pinY = self._label.boundingRect().height() / 2 - self._pin.boundingRect().height() / 2
        self._label.setPos(self.pin().boundingRect().width(), 0)
        self._pin.setPos(0, pinY)
        super(_PlugItem, self).layout()

class OutPlugItem(_PlugItem):
    def __init__(self, name):
        super(OutPlugItem, self).__init__(name)

    def _layout(self):
        pinY = self._label.boundingRect().height() / 2 - self._pin.boundingRect().height() / 2
        self._label.setPos(0, 0)
        self._pin.setPos(self._label.boundingRect().width(), pinY)
        super(_PlugItem, self).layout()
