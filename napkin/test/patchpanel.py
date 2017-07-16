from patch.edgeitem import EdgeItem
from patch.graphscene import GraphScene
from patch.graphview import GraphView
from patch.inoutnodeitem import *
from patch.nodeitem import SocketItem
from patch.operator import *
from utils.qtutils import randomTypeColor


def dataTypeCondition(src: SocketItem, dst: SocketItem):
    if isinstance(src, OutputSocketItem):
        outlet, inlet = src.outlet, dst.inlet
    else:
        outlet, inlet = dst.outlet, src.inlet

    if isinstance(outlet, TriggerOutlet) and isinstance(inlet, TriggerInlet):
        return True

    if isinstance(outlet, DataOutlet) and isinstance(inlet, DataInlet):
        return bool(typeConverter(outlet.valueType, inlet.valueType))

    return False


class OperatorItem(InputOutputNodeItem):
    def __init__(self, op: Operator):
        super(OperatorItem, self).__init__(op.name())
        self.operator = op

        for inlet in self.operator.inlets():
            socket = self.addInlet(inlet.name())
            socket.inlet = inlet
            if isinstance(inlet, DataInlet):
                socket.setToolTip('%s (%s)' % (inlet.name(), inlet.valueType.__name__))
                socket.setPinColor(randomTypeColor(inlet.valueType))
            elif isinstance(inlet, TriggerInlet):
                socket.setToolTip('%s <trigger>' % inlet.name())

        for outlet in self.operator.outlets():
            socket = self.addOutlet(outlet.name())
            socket.outlet = outlet
            if isinstance(outlet, DataOutlet):
                socket.setToolTip('%s (%s)' % (outlet.name(), outlet.valueType.__name__))
                socket.setPinColor(randomTypeColor(outlet.valueType))
            elif isinstance(outlet, TriggerOutlet):
                socket.setToolTip('%s <trigger>' % outlet.name())


class PatchScene(GraphScene):
    def __init__(self):
        super(PatchScene, self).__init__()
        self.patch = Patch()
        self.addConnectCondition(dataTypeCondition)

    def createOperator(self, opType, pos):
        op = opType()
        op.meta.x = pos.x()
        op.meta.y = pos.y()
        item = OperatorItem(op)
        item.setPos(QPointF(op.meta.x, op.meta.y))
        self.patch.addOperator(op)
        self.addNode(item)

    def __findEdge(self, outlet: Outlet, inlet: Inlet):
        for e in self.edges():
            if e.srcSocket.outlet == outlet and e.dstSocket.inlet == inlet:
                return e

    def addEdge(self, src: SocketItem, dst: SocketItem):
        if isinstance(src, InputSocketItem):
            src, dst = dst, src

        outlet = src.outlet
        inlet = dst.inlet
        # remove existing connection first
        if isinstance(inlet, DataInlet):
            if inlet.connectedOutlet:
                oldEdge = self.__findEdge(inlet.connectedOutlet, inlet)
                self.removeItem(oldEdge)
            inlet.connect(outlet)
        elif isinstance(outlet, TriggerOutlet):
            if outlet.connectedInlet:
                oldEdge = self.__findEdge(outlet, outlet.connectedInlet)
                self.removeItem(oldEdge)
            outlet.connect(inlet)
        super(PatchScene, self).addEdge(src, dst)

    def removeItem(self, item: QGraphicsItem):
        if isinstance(item, EdgeItem):
            if isinstance(item.dstSocket.inlet, DataInlet):
                item.dstSocket.inlet.disconnect()
            elif isinstance(item.srcSocket.outlet, TriggerOutlet):
                item.srcSocket.outlet.disconnect()

        super(PatchScene, self).removeItem(item)


class PatchView(GraphView):
    def __init__(self):
        super(PatchView, self).__init__()
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self.__onCustomContextMenuRequested)

    def __onCustomContextMenuRequested(self, pos):
        menu = QMenu()
        for opType in allOperators():
            def f(enabled, opType=opType, pos=pos):
                self.__onOperatorCreate(opType, pos)

            menu.addAction(opType.displayName).triggered.connect(f)
        menu.exec_(self.mapToGlobal(pos))

    def __onOperatorCreate(self, opType, viewPos):
        self.scene().createOperator(opType, self.mapToScene(viewPos))


class PatchPanel(QWidget):
    def __init__(self):
        super(PatchPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.graphView = PatchView()
        self.layout().addWidget(self.graphView)
        self.graphView.setScene(PatchScene())

    def patch(self):
        return self.scene().patch

    def scene(self) -> PatchScene:
        return self.graphView.scene()
