import math

from PyQt5.QtCore import pyqtSignal, Qt, QPointF
from PyQt5.QtGui import QPen
from PyQt5.QtWidgets import QGraphicsScene, QApplication

from patch.layeritem import LayerItem
from patch.operatoritem import OperatorItem
from patch.patchutils import _getObjectEditorPos
from patch.wireitem import WireItem
from patch.wirepreview import WirePreview


class PatchScene(QGraphicsScene):
    operatorSelectionChanged = pyqtSignal(list)

    def __init__(self, ctx, patch):
        """
        @type ctx: AppContext
        """
        self.ctx = ctx
        assert (patch)
        super(PatchScene, self).__init__()
        self.__wireIsOutput = False
        self.__patch = patch

        self.__operatorLayer = LayerItem()
        self.addItem(self.__operatorLayer)
        self.__wireLayer = LayerItem()
        self.addItem(self.__wireLayer)
        self.__interactionLayer = LayerItem()
        self.__interactionLayer.setAcceptedMouseButtons(Qt.NoButton)
        self.__interactionLayer.setAcceptHoverEvents(False)
        self.__interactionLayer.setAcceptTouchEvents(False)
        self.__interactionLayer.setEnabled(False)
        self.addItem(self.__interactionLayer)

        self.__previewWire = WirePreview(self.__interactionLayer)
        self.__previewWire.setEnabled(False)
        self.__previewWire.setParentItem(self.__interactionLayer)
        self.__previewWire.setVisible(False)

        self.__isWiring = False

        # self.setSceneRect(-1000, -1000, 1000, 1000)
        self.selectionChanged.connect(self.__onSelectionChanged)
        self.__patch.childAdded.connect(self.__onOperatorAdded)
        self.__patch.childRemoved.connect(self.__onOperatorRemoved)

        for op in self.__patch.children():
            self.__onOperatorAdded(op)

        for opItem in self.operatorItems():
            for inPlug in opItem.operator().inputPlugs():
                con = inPlug.connection()
                if con:
                    self.__addWire(con, inPlug)

    def startDragConnection(self, pinItem):
        self.hideIncompaticlePlugs(pinItem.plugItem())
        self.__wireIsOutput = not pinItem.plugItem().isInput()
        if self.__wireIsOutput:
            self.__previewWire.srcPin = pinItem
            self.__previewWire.dstPos = pinItem.attachPos()
        else:
            self.__previewWire.dstPin = pinItem
            self.__previewWire.srcPos = pinItem.attachPos()
        self.__previewWire.setVisible(True)
        self.__isWiring = True

    def updateDragConnection(self, pt):
        if self.__wireIsOutput:
            self.__previewWire.dstPos = pt
        else:
            self.__previewWire.srcPos = pt
        self.__previewWire.updatePath()

    def stopDragConnection(self, pinItem):
        if not self.__isWiring:
            return
        if pinItem:
            if self.__wireIsOutput:
                srcPlugItem = self.__previewWire.srcPin.plugItem()
                dstPlugItem = pinItem.plugItem()
            else:
                srcPlugItem = pinItem.plugItem()
                dstPlugItem = self.__previewWire.dstPin.plugItem()

            if srcPlugItem != dstPlugItem:
                srcPlug = srcPlugItem.plug()
                dstPlug = dstPlugItem.plug()
                dstPlug.connectTo(srcPlug)

        self.__previewWire.srcPin = None
        self.__previewWire.dstPin = None
        self.__previewWire.setVisible(False)
        self.__isWiring = False
        self.showAllPlugs()

    def drawBackground(self, painter, rect):
        spacing = 50
        xmin = int(math.floor(rect.left() / spacing))
        xmax = int(math.ceil(rect.right() / spacing))
        ymin = int(math.floor(rect.top() / spacing))
        ymax = int(math.ceil(rect.bottom() / spacing))

        pen = QPen()
        pen.setWidth(0)
        pen.setColor(QApplication.palette().base().color().darker(110))
        painter.setPen(pen)

        for x in range(xmin, xmax):
            painter.drawLine(QPointF(x, ymin) * spacing,
                             QPointF(x, ymax) * spacing)
        for y in range(ymin, ymax):
            painter.drawLine(QPointF(xmin, y) * spacing,
                             QPointF(xmax, y) * spacing)

    def operatorItems(self):
        for item in self.__operatorLayer.childItems():
            if isinstance(item, OperatorItem):
                yield item

    def wireItems(self):
        for item in self.__wireLayer.childItems():
            if isinstance(item, WireItem):
                yield item

    def patch(self):
        return self.__patch

    def hideIncompaticlePlugs(self, src):
        for opItem in self.operatorItems():
            opItem.hideIncompatiblePlugs(src)

    def showAllPlugs(self):
        for opItem in self.operatorItems():
            opItem.showAllPlugs()

    def selectedOperatorItems(self):
        return (item for item in self.selectedItems() if
                isinstance(item, OperatorItem))

    def selectedOperators(self):
        for opItem in self.selectedOperatorItems():
            yield opItem.operator()

    def selectedWires(self):
        return (item for item in self.selectedItems() if
                isinstance(item, WireItem))

    def findOperatorItem(self, op):
        for item in self.operatorItems():
            if item.operator() == op:
                return item

        print('Could not find operator: %s' % op)

    def findWireItem(self, srcPlugItem, dstPlugItem):
        for item in self.wireItems():
            if item.srcPin.plugItem().plug() == srcPlugItem and item.dstPin.plugItem().plug() == dstPlugItem:
                return item

    def dragConnectionSource(self):
        if self.__previewWire.srcPin:
            return self.__previewWire.srcPin.plugItem().plug()
        if self.__previewWire.dstPin:
            return self.__previewWire.dstPin.plugItem().plug()
        return None

    def __onSelectionChanged(self):
        operators = list(self.selectedOperators())
        self.ctx.setSelection(operators)
        self.operatorSelectionChanged.emit(operators)

    def __addWire(self, srcPlug, dstPlug):
        """
        @type srcPlug: nap.OutputPlugBase
        @type dstPlug: nap.InputPlugBase
        """
        srcPlugItem = self.findOperatorItem(srcPlug.parent()).findPlugItem(
            srcPlug)
        dstPlugItem = self.findOperatorItem(dstPlug.parent()).findPlugItem(
            dstPlug)

        wire = WireItem(srcPlugItem.pin(), dstPlugItem.pin())
        wire.setParentItem(self.__wireLayer)

    def __removeWire(self, srcPlug, dstPlug):
        wire = self.findWireItem(srcPlug, dstPlug)
        assert(wire)
        self.__removeItem(wire)

    def __onOperatorAdded(self, op):
        """
        @type op: nap.Operator
        """
        item = OperatorItem(self.__operatorLayer, op)
        item.moved.connect(self.__updateSceneRect)
        item.setParentItem(self.__operatorLayer)
        item.plugConnected.connect(self.__addWire)
        item.plugDisconnected.connect(self.__removeWire)
        item.setPos(_getObjectEditorPos(op))
        self.__updateSceneRect()

    def __onOperatorRemoved(self, op):
        self.__removeItem(self.findOperatorItem(op))

    def __onOperatorChanged(self, op):
        self.findOperatorItem(op).setPos(_getObjectEditorPos(op))

    def __removeItem(self, item):
        self.removeItem(item)
        del item

    def __updateSceneRect(self):
        self.setSceneRect(
            self.itemsBoundingRect().adjusted(-1000, -1000, 1000, 1000))