from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from patch.socketitem import InputSocketItem, OutputSocketItem
from patch.patchutils import COL_NODE_TITLE, moveToFront


class NodeItem(QGraphicsObject):
    def __init__(self):
        super(NodeItem, self).__init__()



class InputOutputNodeItem(NodeItem):
    moved = pyqtSignal()
    plugConnected = pyqtSignal(object, object)
    plugDisconnected = pyqtSignal(object, object)

    def __init__(self, name:str):
        super(InputOutputNodeItem, self).__init__()
        self.__inputPlugs = []
        self.__outputPlugs = []
        self.__titleLabel = QGraphicsTextItem(self)
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)

        self.__operatorBorder = 1
        self.__operatorBorderSelected = 2

        self.setTitle(name)

    def inputPlugs(self):
        return self.__inputPlugs

    def outputPlugs(self):
        return self.__outputPlugs

    def plugs(self):
        return self.__inputPlugs + self.__outputPlugs

    def boundingRect(self):
        b = self.__operatorBorder
        return self.childrenBoundingRect().adjusted(-b, -b, b, b)

    def paint(self, painter, option, widget=None):
        painter.setBrush(COL_NODE_TITLE)
        painter.drawRect(self.__titleRect)
        painter.setBrush(QApplication.palette().window())
        if self.isSelected():
            painter.setPen(QPen(QApplication.palette().highlight(), 0))
        else:
            painter.setPen(QPen(QApplication.palette().dark(), 0))
        painter.drawRect(self.boundingRect())

        headerRect = QRectF(0, 0, self.boundingRect().width(),
                            self.__titleRect.height())
        painter.setBrush(QApplication.palette().window().color().darker(110))
        painter.setPen(QPen(Qt.NoPen))
        painter.drawRect(headerRect.adjusted(0, 0, -2, -1))

    def setTitle(self, name):
        self.__titleLabel.setPlainText(name)
        self.layout()

    def findPlugItem(self, plug):
        for item in self.plugs():
            if item.plug() == plug:
                return item

    def name(self):
        return str(self.__titleLabel.toPlainText())

    def hideIncompatiblePlugs(self, src):
        for plug in self.plugs():
            if plug == src:
                continue
            if not _canConnect(plug.plug(), src.plug()):
                plug.setUsable(False)

    def showAllPlugs(self):
        for plug in self.plugs():
            plug.setUsable(True)

    def layout(self):
        headerHeight = 20
        headerSpacing = 6
        itemSpacing = 14
        currentX = 0

        for i in range(len(self.__inputPlugs)):
            item = self.__inputPlugs[i]
            item.setPos(0, headerHeight + headerSpacing + i * itemSpacing)
            currentX = max(currentX, item.boundingRect().width())
        maxX = currentX
        for i in range(len(self.__outputPlugs)):
            maxX = max(maxX,
                       currentX + self.__outputPlugs[i].boundingRect().width())

        self.__titleRect = self.__titleLabel.boundingRect()
        self.__titleRect.setWidth(self.childrenBoundingRect().width())
        self.__titleLabel.setPos(
            self.__titleRect.right() - self.__titleLabel.boundingRect().width(),
            0)

        maxX = max(maxX, self.__titleRect.width())

        for i in range(len(self.__outputPlugs)):
            item = self.__outputPlugs[i]
            item.setPos(maxX - item.boundingRect().width(),
                        headerHeight + headerSpacing + i * itemSpacing)

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemSelectedHasChanged:
            moveToFront(self)
        if change == QGraphicsItem.ItemPositionHasChanged:
            self.moved.emit()
        return super(InputOutputNodeItem, self).itemChange(change, value)

    def addInlet(self, name:str):
        plug = InputSocketItem(name)
        plug.setParentItem(self)
        self.__inputPlugs.append(plug)
        self.layout()

    def addOutlet(self, name:str):
        plug = OutputSocketItem(name)
        plug.setParentItem(self)
        self.__outputPlugs.append(plug)
        self.layout()

    def removePlug(self, plug):
        item = self.findPlugItem(plug)
        del item

