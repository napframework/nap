from PyQt5.QtCore import pyqtSignal, QRectF, Qt
from PyQt5.QtGui import QPen
from PyQt5.QtWidgets import QGraphicsObject, QGraphicsTextItem, QGraphicsItem, QApplication

import nap
from patch.plugitem import PlugItem
from patch.patchutils import COL_NODE_TITLE, _canConnect, moveToFront, _getObjectEditorPos


class OperatorItem(QGraphicsObject):
    moved = pyqtSignal()
    plugConnected = pyqtSignal(object, object)
    plugDisconnected = pyqtSignal(object, object)

    def __init__(self, parent, op):
        super(OperatorItem, self).__init__()
        assert (isinstance(op, nap.Operator))
        self.__inputPlugs = []
        self.__outputPlugs = []
        self.__operator = op
        self.__titleLabel = QGraphicsTextItem(self)
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)

        self.__operatorBorder = 1
        self.__operatorBorderSelected = 2

        self.setTitle(op.name())

        # TODO: Port some extra code from the C++ implementation here

        for p in self.__operator.inputPlugs():
            self.__onInputPlugAdded(p)
        for p in self.__operator.outputPlugs():
            self.__onOutputPlugAdded(p)

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

    def operator(self):
        """
        @rtype: nap.Operator
        """
        return self.__operator

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
        return super(OperatorItem, self).itemChange(change, value)

    def __onInputPlugAdded(self, plug):
        """
        @type plug: nap.InputPlugBase
        """
        plug.connected.connect(self.__onPlugConnected)
        plug.disconnected.connect(self.__onPlugDisconnected)
        plugItem = PlugItem(self, plug)
        self.__inputPlugs.append(plugItem)
        self.layout()

    def __onOutputPlugAdded(self, plug):
        plugItem = PlugItem(self, plug)
        self.__outputPlugs.append(plugItem)
        self.layout()

    def __onPlugRemoved(self, plug):
        item = self.findPlugItem(plug)
        del item

    def __onAttributeChanged(self, attrib):
        # TODO: Review
        self.setPos(_getObjectEditorPos(self.__operator))

    def __onPlugConnected(self, srcPlug, dstPlug):
        self.plugConnected.emit(srcPlug, dstPlug)

    def __onPlugDisconnected(self, srcPlug, dstPlug):
        self.plugDisconnected.emit(srcPlug, dstPlug)