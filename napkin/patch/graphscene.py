import math
from typing import Iterable

from patch.edgeitem import *
from patch.inoutnodeitem import *
from patch.layeritem import *
from patch.nodeitem import *

_typeFilter = lambda m, t: isinstance(m, t)


def inputOutputConnectCondition(src: SocketItem, dst: SocketItem):
    if isinstance(src, InputSocketItem) and isinstance(dst, InputSocketItem):
        return False
    if isinstance(src, OutputSocketItem) and isinstance(dst, OutputSocketItem):
        return False
    return True


class GraphScene(QGraphicsScene):
    nodeSelectionChanged = pyqtSignal(list)

    def __init__(self):
        """
        @type ctx: AppContext
        """
        super(GraphScene, self).__init__()

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

        self.__overlayer = LayerItem()
        self.__overlayer.setZValue(-10000)
        self.addItem(self.__overlayer)

        self.__previewWire = PreviewEdge()
        self.__previewWire.setEnabled(False)
        self.__previewWire.setVisible(False)
        self.__previewWire.setParentItem(self.__overlayer)

        self.__connectConditions = [inputOutputConnectCondition]

        self.__isWiring = False

        # self.setSceneRect(-1000, -1000, 1000, 1000)
        self.selectionChanged.connect(self.__onSelectionChanged)

        for opItem in self.nodes():
            for inPlug in opItem.operator().inputSockets():
                con = inPlug.connection()
                if con:
                    self.addEdge(con, inPlug)

        self.changed.connect(self.__sceneChanged)

    def addConnectCondition(self, fn):
        self.__connectConditions.append(fn)

    def __sceneChanged(self, regions):
        self.ensureLargeEnoughSceneRect()

    def itemMoved(self, node):
        for edge in self.findEdges(node):
            edge.update()

    def ensureLargeEnoughSceneRect(self):
        """Enlarge scenerect so dragging nodes around doesn't feel so weird"""
        margin = 1000
        adjusted = self.itemsBoundingRect().adjusted(-margin, -margin, margin, margin)
        self.setSceneRect(self.sceneRect().united(adjusted))

    def addNode(self, node: NodeItem):
        node.setParentItem(self.__nodeLayer)

    def canConnect(self, src: SocketItem, dst: SocketItem):
        for cond in self.__connectConditions:
            if not cond(src, dst):
                return False
        return True

    def startDragConnection(self, socket):
        self.hideIncompatiblePlugs(socket)
        self.__previewWire.srcSocket = socket
        self.__previewWire.dstPos = socket.attachPosVec()
        self.__previewWire.setVisible(True)
        self.__isWiring = True

    def updateDragConnection(self, pos, vec):
        self.__previewWire.dstPos = pos
        self.__previewWire.dstVec = vec
        self.__previewWire.updatePath()

    def stopDragConnection(self, dstSocket: SocketItem):
        if not self.__isWiring:
            return
        if dstSocket:
            srcSocket = self.__previewWire.srcSocket
            self.addEdge(srcSocket, dstSocket)

        self.__previewWire.srcSocket = None
        self.__previewWire.dstSocket = None
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
            painter.drawLine(QPointF(x, ymin) * spacing, QPointF(x, ymax) * spacing)
        for y in range(ymin, ymax):
            painter.drawLine(QPointF(xmin, y) * spacing, QPointF(xmax, y) * spacing)

    def nodes(self) -> Iterable[NodeItem]:
        return filter(lambda m: isinstance(m, NodeItem), self.__nodeLayer.childItems())

    def edges(self) -> Iterable[EdgeItem]:
        return filter(lambda m: isinstance(m, EdgeItem), self.__edgeLayer.childItems())

    def hideIncompatiblePlugs(self, src: SocketItem):
        for node in self.nodes():
            for dst in node.sockets():
                if not self.canConnect(src, dst):
                    dst.setUsable(False)
        src.setUsable(True)

    def showAllPlugs(self):
        for node in self.nodes():
            node.showAllPlugs()

    def selectedNodes(self) -> Iterable[NodeItem]:
        return filter(lambda m: isinstance(m, NodeItem), self.selectedItems())

    def selectedEdges(self) -> Iterable[EdgeItem]:
        return filter(lambda m: isinstance(m, EdgeItem), self.selectedItems())

    def findEdge(self, plugA: SocketItem, plugB: SocketItem):
        for item in self.edges():
            if item.srcPin.plugItem() == plugA and item.dstPin.plugItem() == plugB:
                return item
            if item.dstPin.plugItem() == plugB and item.dstPin.plugItem() == plugA:
                return item

    def findEdges(self, node):
        for edge in self.edges():
            if edge.dstSocket.node() == node:
                yield edge
            if edge.srcSocket.node() == node:
                yield edge

    def dragConnectionSource(self) -> SocketItem:
        if self.__previewWire.srcSocket:
            return self.__previewWire.srcSocket
        if self.__previewWire.dstSocket:
            return self.__previewWire.dstSocket
        return None

    def __onSelectionChanged(self):
        selection = list(self.selectedNodes())
        self.__moveNodesToTop(selection)

    def __moveNodesToTop(self, nodes):
        startindex = 0
        for z, node in enumerate(self.nodes()):
            node.setZValue(z)
            startindex = z
        for z, node in enumerate(nodes):
            node.setZValue(startindex + z)

        self.nodeSelectionChanged.emit(list(self.selectedNodes()))


    def addEdge(self, src: SocketItem, dst: SocketItem):
        wire = EdgeItem(src, dst)
        wire.setParentItem(self.__edgeLayer)

    def removeEdge(self, srcPlug, dstPlug):
        wire = self.findEdge(srcPlug, dstPlug)
        assert (wire)
        self.__removeItem(wire)

    def __removeItem(self, item):
        self.removeItem(item)
        del item
