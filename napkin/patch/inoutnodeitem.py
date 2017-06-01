from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from patch.nodeitem import SocketItem, NodeItem, PinItem
from patch.patchutils import moveToFront, Margins


class InputOutputNodeItem(NodeItem):
    moved = pyqtSignal()
    plugConnected = pyqtSignal(object, object)
    plugDisconnected = pyqtSignal(object, object)

    def __init__(self, name: str):
        super(InputOutputNodeItem, self).__init__()
        # :type List[SocketItem]
        self.__inputSockets = []
        # :type List[SocketItem]
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
        hSpacing = 14

        # input plugs
        maxInSocks = len(self.__inputSockets)
        maxOutSocks = len(self.__outputSockets)
        maxitems = max(maxInSocks, maxOutSocks)

        maxWidth = self.__titleLabel.boundingRect().width()

        # brick-stack in and out sockets
        y = self.__titleLabel.boundingRect().bottom()
        for i in range(maxitems):
            x = 0
            if i < maxInSocks:
                sock = self.__inputSockets[i]
                sock.layout()
                sock.setPos(x, y)
                x += sock.boundingRect().width() + hSpacing
            if i < maxOutSocks:
                sock = self.__outputSockets[i]
                sock.layout()
                sock.setPos(x, y)
                x += sock.boundingRect().width()
            maxWidth = max(maxWidth, x)
            y += itemSpacing

        self.__titleLabel.setTextWidth(maxWidth)

        # right align output sockets
        for sock in self.__outputSockets:
            x = maxWidth - sock.boundingRect().width()
            y = sock.pos().y()
            sock.setPos(x, y)

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


class TitleItem(QGraphicsTextItem):
    def __init__(self):
        super(TitleItem, self).__init__()
    
    def paint(self, painter:QPainter, option:QStyleOptionGraphicsItem, widget:QWidget):
        painter.fillRect(self.boundingRect(), QBrush(QColor('#DDDDDD')))
        super(TitleItem, self).paint(painter, option, widget)
        
    
    #     self.__font = QApplication.font()
    #     self.__fontMetrics = QFontMetricsF(self.__font)
    #     self.__text = 'Unknown'
    #     self.__rect = QRectF()
    #     self.__textPen = QPen(QColor(QApplication.instance().palette().windowText()))
    #     self.__textRect = QRectF()
    #     self.__preferredWidth = 0
    #     self.__dirty = True
    #     self.__textMargins = Margins(top=2, right=4, bottom=4, left=4)
    #
    # def __updateMetrics(self):
    #     if not self.__dirty:
    #         return
    #     self.__textRect = self.__fontMetrics.boundingRect(self.__text)
    #     self.__textRect.setWidth(self.__preferredWidth)
    #     self.__textRect.adjust(self.__textMargins.left, self.__textMargins.top,
    #                            self.__textMargins.left, self.__textMargins.top)
    #     self.__rect = self.__textRect.adjusted(-self.__textMargins.left, -self.__textMargins.top,
    #                                            self.__textMargins.right, self.__textMargins.bottom)
    #     self.__dirty = False
    #
    #
    # def setPlainText(self, text):
    #     self.__text = text
    #     self.__dirty = True
    #
    # def boundingRect(self):
    #     self.__updateMetrics()
    #     return self.__rect
    #
    # def paint(self, painter: QPainter, option: QStyleOptionGraphicsItem, widget: QWidget):
    #     self.__updateMetrics()
    #     painter.setPen(Qt.NoPen)
    #     painter.fillRect(self.__rect, QBrush(QColor('#DDDDDD')))
    #     painter.setPen(self.__textPen)
    #     painter.drawText(self.__textRect, Qt.AlignLeft, self.__text)
    #
    # def setPreferredWidth(self, w):
    #     self.__preferredWidth = w
    #     self.__dirty = True

class InputSocketItem(SocketItem):
    def __init__(self, name):
        super(InputSocketItem, self).__init__(name)
        self._pin = PinItem(self)
        self.__vec = QPointF(-1, 0)

    def layout(self):
        pinY = self._label.boundingRect().height() / 2 - self._pin.boundingRect().height() / 2
        self._label.setPos(self._pin.boundingRect().width(), 0)
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
        self._pin = PinItem(self)
        self.__vec = QPointF(1, 0)

    def layout(self):
        pinY = self._label.boundingRect().height() / 2 - self._pin.boundingRect().height() / 2
        self._label.setPos(0, 0)
        self._pin.setPos(self._label.boundingRect().width(), pinY)
        if self.scene():
            self.scene().update()

    def attachPosVec(self) -> (QPointF, QPointF):
        r = self._pin.boundingRect()
        pos = self._pin.mapToScene(QPointF(r.right(), r.top() + r.height() / 2))
        return pos, self.__vec


