from PyQt5.QtCore import *
from PyQt5.QtCore import QCoreApplication
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from patch.socketitem import InputSocketItem, OutputSocketItem
from patch.patchutils import COL_NODE_TITLE, moveToFront


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


class InputOutputNodeItem(NodeItem):
    moved = pyqtSignal()
    plugConnected = pyqtSignal(object, object)
    plugDisconnected = pyqtSignal(object, object)

    def __init__(self, name: str):
        super(InputOutputNodeItem, self).__init__()
        self.__inputSockets = []
        self.__outputSockets = []
        self.__titleLabel = TitleItem()
        self.__titleLabel.setParentItem(self)
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)

        self.__operatorBorder = 1
        self.__operatorBorderSelected = 2

        self.setName(name)

    def inputSockets(self):
        return self.__inputSockets

    def outputSockets(self):
        return self.__outputSockets

    def sockets(self):
        for sock in self.__inputSockets:
            yield sock
        for sock in self.__outputSockets:
            yield sock

    def boundingRect(self):
        b = self.__operatorBorder
        return self.childrenBoundingRect().adjusted(-b, -b, b, b)

    def paint(self, painter, option, widget=None):
        painter.setBrush(QApplication.palette().window())
        if self.isSelected():
            painter.setPen(QPen(QApplication.palette().highlight(), 0))
        else:
            painter.setPen(QPen(QApplication.palette().dark(), 0))
        painter.drawRect(self.boundingRect())

    def setName(self, name):
        self.__titleLabel.setPlainText(name)
        self.layout()

    def findPlugItem(self, plug):
        for item in self.sockets():
            if item.plug() == plug:
                return item

    def name(self):
        return str(self.__titleLabel.toPlainText())

    def showAllPlugs(self):
        for plug in self.sockets():
            plug.setVisible(True)

    def layout(self):
        headerHeight = 20
        headerSpacing = 6
        itemSpacing = 14
        maxwidth = 0

        # input plugs


        # for i in range(len(self.__inputPlugs)):
        #
        for i, item in enumerate(self.__inputSockets):
            x = 0
            y = headerHeight + headerSpacing + i * itemSpacing
            item.setPos(x, y)
            maxwidth = max(maxwidth, item.boundingRect().width())

        maxX = maxwidth
        for i, item in enumerate(self.__outputSockets):
            maxX = max(maxX,
                       maxwidth + self.__outputSockets[i].boundingRect().width())

        for i, item in enumerate(self.__outputSockets):
            item.setPos(maxX - item.boundingRect().width(),
                        headerHeight + headerSpacing + i * itemSpacing)

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemSelectedHasChanged:
            moveToFront(self)
        if change == QGraphicsItem.ItemPositionHasChanged:
            self.moved.emit()
        return super(InputOutputNodeItem, self).itemChange(change, value)

    def addInlet(self, name: str):
        plug = InputSocketItem(name)
        plug.setParentItem(self)
        self.__inputSockets.append(plug)
        self.layout()

    def addOutlet(self, name: str):
        plug = OutputSocketItem(name)
        plug.setParentItem(self)
        self.__outputSockets.append(plug)
        self.layout()

    def removePlug(self, plug):
        item = self.findPlugItem(plug)
        del item


class TitleItem(QGraphicsItem):
    def __init__(self):
        super(TitleItem, self).__init__()
        self.__font = QApplication.font()
        self.__fontMetrics = QFontMetricsF(self.__font)
        self.__text = 'Unknown'
        self.__rect = QRectF()

    def setPlainText(self, text):
        self.__text = text
        self.__rect = self.__fontMetrics.boundingRect(self.__text)

    def boundingRect(self):
        return self.__rect

    def paint(self, painter: QPainter, option: QStyleOptionGraphicsItem, widget: QWidget):
        painter.drawText(self.__rect, Qt.AlignLeft, self.__text)