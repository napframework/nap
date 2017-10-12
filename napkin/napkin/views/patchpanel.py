from generic.qtutils import randomTypeColor
from napkin.models.typeconversion import typeConverter
from napkin.views.graphscene import GraphScene
from napkin.views.graphview import GraphView
from napkin.views.edgeitem import EdgeItem
from napkin.views.inoutnodeitem import *
from napkin.views.nodeitem import SocketItem
from napkin.models.nap import *


def haveConvertibleTypes(src: SocketItem, dst: SocketItem):
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
        self.addConnectCondition(haveConvertibleTypes)

    def createOperator(self, name, opType, pos):
        assert opType
        op = opType()
        assert op
        op.x(pos.x())
        op.y(pos.y())
        item = OperatorItem(op)
        item.setPos(QPointF(op.x(), op.y()))
        self.patch.addChild(op)
        self.addNode(item)

    def __findEdge(self, outlet: Outlet, inlet: Inlet):
        for e in self.edges():
            if e.srcSocket.outlet == outlet and e.dstSocket.inlet == inlet:
                return e

    def selectedOperators(self):
        return [n.operator for n in self.selectedNodes()]

    def addEdge(self, src: SocketItem, dst: SocketItem):
        if isinstance(src, InputSocketItem):
            src, dst = dst, src

        outlet = src.outlet
        inlet = dst.inlet
        # remove existing connection first
        if isinstance(inlet, DataInlet):
            if inlet.isConnected():
                oldEdge = self.__findEdge(inlet.connection(), inlet)
                self.removeItem(oldEdge)
            inlet.connect(outlet)
        elif isinstance(outlet, TriggerOutlet):
            if outlet.isConnected():
                oldEdge = self.__findEdge(outlet, outlet.connection())
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


def createOperatorActions(parent, scene, scenePos):
    for opType in operatorTypes():

        def f(enabled, opt=opType, pos=scenePos):
            scene.createOperator(opt.displayName(), opt, pos)

        action = QAction(opType.displayName(), parent)
        action.triggered.connect(f)
        yield action


class CreateOperatorTypeItem(QStandardItem):
    def __init__(self, action):
        super(CreateOperatorTypeItem, self).__init__()
        self.action = action
        self.setText(self.action.text())
        self.setEditable(False)


class CreateOperatorTypeModel(QStandardItemModel):
    def __init__(self, parent, scene, scenePos):
        super(CreateOperatorTypeModel, self).__init__()
        for action in createOperatorActions(parent, scene, scenePos):
            self.appendRow(CreateOperatorTypeItem(action))


class CreateOperatorDialog(QMenu):
    def __init__(self, parent, scene, pos):
        super(CreateOperatorDialog, self).__init__(parent)
        self.setMinimumSize(200, 200)
        self.setLayout(QVBoxLayout())
        self.layout().setSpacing(0)
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.tf = QLineEdit()
        self.tf.textChanged.connect(self.__onFilterChanged)
        # self.layout().addWidget(self.tf)

        self.tree = QTreeView()
        self.tree.setHeaderHidden(True)
        self.tree.setRootIsDecorated(False)
        self.sortFilter = QSortFilterProxyModel()
        self.model = CreateOperatorTypeModel(parent, scene, pos)
        self.sortFilter.setSourceModel(self.model)
        self.sortFilter.setFilterCaseSensitivity(False)
        self.tree.setModel(self.sortFilter)
        self.tree.clicked.connect(self.__onClicked)
        self.layout().addWidget(self.tree)

    def __onFilterChanged(self, text):
        self.sortFilter.setFilterFixedString(text)
        self.tree.selectionModel().setCurrentIndex(self.sortFilter.index(0, 0),
                                                   QItemSelectionModel.ClearAndSelect)

    def keyPressEvent(self, evt: QKeyEvent):
        if evt.key() == Qt.Key_Enter or evt.key() == Qt.Key_Return:
            self.applyAction()
        return super(CreateOperatorDialog, self).keyPressEvent(evt)

    def __onClicked(self):
        self.applyAction()

    def applyAction(self):
        idx = self.tree.selectionModel().selectedIndexes()
        if not idx: return
        idx = idx[0]
        item = self.model.itemFromIndex(self.sortFilter.mapToSource(idx))
        item.action.trigger()
        self.close()

    def showEvent(self, evt):
        self.tree.setFocus()


class PatchView(GraphView):
    def __init__(self):
        super(PatchView, self).__init__()
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(
            self.__onCustomContextMenuRequested)

    def __onCustomContextMenuRequested(self, pos):
        self.showOperatorDialog(pos)

    def showOperatorDialog(self, pos):
        dialog = CreateOperatorDialog(self, self.scene(), self.mapToScene(pos))
        dialog.move(self.mapToGlobal(pos))
        dialog.exec_()

    def showRegularMenu(self, pos):
        menu = QMenu()
        for action in createOperatorActions(menu, self.scene(),
                                            self.mapToScene(pos)):
            menu.addAction(action)
        menu.exec_(self.mapToGlobal(pos))


class PatchPanel(QWidget):
    selectionChanged = pyqtSignal(list)

    def __init__(self):
        super(PatchPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.graphView = PatchView()
        self.layout().addWidget(self.graphView)

        self.scene = PatchScene()
        self.graphView.setScene(self.scene)
        self.scene.selectionChanged.connect(self.__onSelectionChanged)

    def __onSelectionChanged(self, *args):
        self.selectionChanged.emit(self.scene.selectedOperators())

    def patchView(self) -> PatchView:
        return self.graphView

    def patch(self) -> Patch:
        return self.scene.patch

    def scene(self) -> PatchScene:
        return self.graphView.scene()
