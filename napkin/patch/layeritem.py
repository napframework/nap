from PyQt5.QtWidgets import QGraphicsItem


class LayerItem(QGraphicsItem):
    def __init__(self):
        super(LayerItem, self).__init__()

    def boundingRect(self):
        return self.childrenBoundingRect()

    def paint(self, painter, option, widget=None):
        pass