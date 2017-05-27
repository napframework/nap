import sys
import typing

import math
from collections import namedtuple

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

from utils.qtutils import QBaseWindow

Margins = namedtuple('Margins', ['top', 'right', 'bottom', 'left'])


class InletItem(QGraphicsItem):
    def __init__(self, name):
        super(InletItem, self).__init__()
        self.name = name
        self.textItem = QGraphicsTextItem(name)

    def boundingRect(self):
        return self.textItem.boundingRect()

    def paint(self, painter: QPainter, option: 'QStyleOptionGraphicsItem',
              widget: typing.Optional[QWidget] = ...):
        pass


class OutletsItem(QGraphicsRectItem):
    PIN_RADIUS = 12

    def __init__(self):
        super(OutletsItem, self).__init__()
        self.spacingX = 16
        self.spacingY = -2
        self.margins = Margins(0, 4, 0, 4)  # T, R, B, L

        self.inlets = []
        self.outlets = []

    def addInlet(self, name):
        # item = InletItem(name)
        item = QGraphicsTextItem(name)

        pin = QGraphicsEllipseItem(-self.PIN_RADIUS, 0, self.PIN_RADIUS, self.PIN_RADIUS)
        pin.setPos(0, item.boundingRect().height() / 2 - self.PIN_RADIUS / 2)
        pin.setParentItem(item)

        item.setParentItem(self)
        self.inlets.append(item)

    def addOutlet(self, name):
        item = QGraphicsTextItem(name)
        item.setParentItem(self)
        self.outlets.append(item)

    def maxWidth(self):
        return self.__calculateContentSize()

    def __calculateContentSize(self):
        maxwidth = 0
        maxheight = 0
        for i in range(max(len(self.inlets), len(self.outlets))):
            x = self.margins.left
            y = self.margins.top
            if i < len(self.inlets):
                item = self.inlets[i]
                assert isinstance(item, QGraphicsItem)
                x += item.boundingRect().width() + self.spacingX
                y += item.boundingRect().height() + self.spacingY

            maxheight = max(maxheight, y)

            y = 0
            if i < len(self.outlets):
                item = self.outlets[i]
                assert isinstance(item, QGraphicsItem)
                x += item.boundingRect().width()
                y += item.boundingRect().height() + self.spacingY

            maxwidth = max(maxwidth, x)
            maxheight = max(maxheight, y)

        return maxwidth, maxheight

    def layout(self):
        y = self.margins.top

        maxwidth, maxheight = self.__calculateContentSize()
        maxheight = 0
        for item in self.inlets:
            item.setPos(self.margins.left, y)
            rect = item.boundingRect()
            y += rect.height() + self.spacingY
            maxheight = y

        y = self.margins.top
        for item in self.outlets:
            rect = item.boundingRect()
            x = maxwidth - rect.width() - self.margins.right
            item.setPos(x, y)
            y += rect.height() + self.spacingY
            maxheight = max(maxheight, y)

        maxheight += self.margins.bottom
        rect = self.rect()
        rect.setWidth(maxwidth)
        rect.setHeight(maxheight)
        self.setRect(rect)

    def contentsBounds(self):
        x, y = self.__calculateContentSize()
        return QSize(x, y)


class TitleWidget(QGraphicsWidget):
    def __init__(self, text):
        super(TitleWidget, self).__init__()
        self.text = text
        self.textMargin = Margins(2, 4, 2, 4)
        self.width = None

    def boundingRect(self):
        return QRectF(self.pos(), QSizeF(self.__textBounds()))

    def contentsBounds(self):
        return QRectF(self.pos(), QSizeF(self.__textBounds()))

    def __textBounds(self):
        metrics = QFontMetrics(self.font())
        size = metrics.size(Qt.TextSingleLine, self.text)
        w = size.width() + self.textMargin.left + self.textMargin.right
        if self.width:
            w = max(self.width, w)
        size.setWidth(w)
        size.setHeight(size.height() + self.textMargin.top + self.textMargin.bottom)
        return size

    def paint(self, painter: QPainter, option: QStyleOptionGraphicsItem, widget: typing.Optional[QWidget] = ...):
        option = QStyleOptionButton()
        option.text = self.text
        option.rect.setSize(self.__textBounds())
        self.style().drawControl(QStyle.CE_PushButton, option, painter)


class OperatorItem(QGraphicsWidget):
    def __init__(self, name):
        super(OperatorItem, self).__init__()
        self.title = TitleWidget(name)
        self.title.setParentItem(self)

        self.outletsItem = OutletsItem()
        self.outletsItem.setPos(0, self.title.boundingRect().height())
        self.outletsItem.setParentItem(self)

    def addInlet(self, name):
        self.outletsItem.addInlet(name)
        self.__layout()

    def addOutlet(self, name):
        self.outletsItem.addOutlet(name)
        self.__layout()

    def __layout(self):
        self.title.width, _ = self.outletsItem.maxWidth()
        self.outletsItem.layout()

    def __contentsBounds(self):
        self.title.contentsBounds().united(self.outletsItem.contentsBounds())

    def paint(self, painter: QPainter, option: QStyleOptionGraphicsItem, widget: typing.Optional[QWidget] = ...):
        option = QStyleOptionButton()
        option.text = self.text
        option.rect.setSize(self.__textBounds())
        self.style().drawControl(QStyle.PE_FrameDefaultButton, option, painter)


class PatchScene(QGraphicsScene):
    def __init__(self):
        super(PatchScene, self).__init__()


class PatchView(QWidget):
    def __init__(self):
        super(PatchView, self).__init__()

        self.__graphicsView = QGraphicsView()
        self.__scene = PatchScene()
        self.__graphicsView.setScene(self.__scene)

        self.setLayout(QVBoxLayout())
        self.layout().addWidget(self.__graphicsView)

    def scene(self):
        return self.__scene





if __name__ == '__main__':
    app = QApplication(sys.argv)

    win = QBaseWindow()

    view = PatchView()

    op = OperatorItem('SimpleOperator')
    op.addInlet('MyInlet')
    op.addInlet('AnotherInput With Long Name')
    op.addOutlet('out')

    view.scene().addItem(op)

    win.setCentralWidget(QWidget())
    win.centralWidget().setLayout(QVBoxLayout())
    win.centralWidget().layout().addWidget(view)

    win.centralWidget().layout().addWidget(QPushButton('Le Butten'))

    win.show()

    app.exec_()
