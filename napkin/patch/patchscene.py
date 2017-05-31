import math

from PyQt5.QtCore import pyqtSignal, Qt, QPointF
from PyQt5.QtGui import QPen
from PyQt5.QtWidgets import QGraphicsScene, QApplication

from patch.layeritem import LayerItem
from patch.inputoutputnodeitem import InputOutputNodeItem, NodeItem
from patch.patchutils import _getObjectEditorPos
from patch._plugitem import _PlugItem
from patch.edgeitem import EdgeItem
from patch.wirepreview import WirePreview

_typeFilter = lambda m, t: isinstance(m, t)


class PatchScene(QGraphicsScene):
    nodeSelectionChanged = pyqtSignal(list)

    def __init__(self):
        """
        @type ctx: AppContext
        """
        super(PatchScene, self).__init__()
        self.__wireIsOutput = False

        self.__nodeLayer = LayerItem()
        self.addItem(self.__nodeLayer)
        self.__edgeLayer = LayerItem()
        self.addItem(self.__edgeLayer)
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

        for opItem in self.nodes():
            for inPlug in opItem.operator().inputPlugs():
                con = inPlug.connection()
                if con:
                    self.addEdge(con, inPlug)

        self.changed.connect(self.__sceneChanged)

    def __sceneChanged(self, regions):
        self.ensureLargeEnoughSceneRect()

    def ensureLargeEnoughSceneRect(self):
        """Enlarge scenerect so dragging nodes around doesn't feel so weird"""
        margin = 1000
        adjusted = self.itemsBoundingRect().adjusted(-margin, -margin, margin, margin)
        self.setSceneRect(self.sceneRect().united(adjusted))

    def addNode(self, node: NodeItem):
        self.addItem(node)

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

    def nodes(self):
        return filter(lambda m: isinstance(m, NodeItem),
                      self.__edgeLayer.childItems())

    def edges(self):
        return filter(lambda m: isinstance(m, EdgeItem),
                      self.__edgeLayer.childItems())

    def hideIncompaticlePlugs(self, src):
        for opItem in self.nodes():
            opItem.hideIncompatiblePlugs(src)

    def showAllPlugs(self):
        for opItem in self.nodes():
            opItem.showAllPlugs()

    def selectedNodes(self):
        return filter(lambda m: isinstance(m, NodeItem), self.selectedItems())

    def selectedEdges(self):
        return filter(lambda m: isinstance(m, EdgeItem), self.selectedItems())

    def findEdge(self, plugA: _PlugItem, plugB: _PlugItem):
        for item in self.edges():
            if item.srcPin.plugItem() == plugA and item.dstPin.plugItem() == plugB:
                return item
            if item.dstPin.plugItem() == plugB and item.dstPin.plugItem() == plugA:
                return item

    def dragConnectionSource(self):
        if self.__previewWire.srcPin:
            return self.__previewWire.srcPin.plugItem().plug()
        if self.__previewWire.dstPin:
            return self.__previewWire.dstPin.plugItem().plug()
        return None

    def __onSelectionChanged(self):
        self.nodeSelectionChanged.emit(list(self.selectedNodes()))

    def addEdge(self, srcPlug: _PlugItem, dstPlug: _PlugItem):
        srcPlugItem = self.findOperatorItem(srcPlug.parent()).findPlugItem(
            srcPlug)
        dstPlugItem = self.findOperatorItem(dstPlug.parent()).findPlugItem(
            dstPlug)

        wire = EdgeItem(srcPlugItem.pin(), dstPlugItem.pin())
        wire.setParentItem(self.__edgeLayer)

    def removeEdge(self, srcPlug, dstPlug):
        wire = self.findEdge(srcPlug, dstPlug)
        assert (wire)
        self.__removeItem(wire)

    def __onOperatorAdded(self, op):
        """
        @type op: nap.Operator
        """
        item = InputOutputNodeItem(self.__nodeLayer, op)
        item.moved.connect(self.__updateSceneRect)
        item.setParentItem(self.__nodeLayer)
        item.plugConnected.connect(self.addEdge)
        item.plugDisconnected.connect(self.removeEdge)
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
