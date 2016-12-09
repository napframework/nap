import math
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

import iconstore
import nap
from appcontext import AppContext

COL_NODE_CONNECTION = QColor(0x70, 0x70, 0x70)
COL_NODE_TITLE = QColor(0xD0, 0xD0, 0xD0)

PATCH_XPOS = '__patch_x_pos__'
PATCH_YPOS = '__patch_y_pos__'


def _dataTypeColor(typeName):
    return Qt.yellow


def _getObjectEditorPos(obj):
    x = obj.attr(PATCH_XPOS).value()
    y = obj.attr(PATCH_YPOS).value()
    return QPointF(x, y)


def _setObjectEditorPos(obj, pos):
    """
    @type obj: nap.AttributeObject
    """
    obj.attr(PATCH_XPOS).setValue(pos.x())
    obj.attr(PATCH_YPOS).setValue(pos.y())


def _clamp(n, minVal, maxVal):
    if n < minVal:
        return minVal
    if n > maxVal:
        return maxVal
    return n


def _filter(items, itemType):
    for item in items:
        if isinstance(item, itemType):
            yield item


def _canConnect(srcPlug, destPlug):
    srcIsInput = isinstance(srcPlug, nap.InputPlugBase)
    destIsInput = isinstance(destPlug, nap.InputPlugBase)
    if srcIsInput == destIsInput:
        return False
    if srcPlug.parent() == destPlug.parent():
        return False
    return srcPlug.dataType() == destPlug.dataType()


def calculateWirePath(srcPos, dstPos, p):
    """
    @type srcPos: QPointF
    @type dstPos: QPointF
    @type p: QPainterPath
    """
    p.moveTo(srcPos)
    if dstPos.x() > srcPos.x():
        hx = srcPos.x() + (dstPos.x() - srcPos.x()) / 2.0
        c1 = QPointF(hx, srcPos.y())
        c2 = QPointF(hx, dstPos.y())
        p.cubicTo(c1, c2, dstPos)
    else:
        maxDist = 150

        dist = _clamp(srcPos.x() - dstPos.x(), -maxDist, maxDist)

        c1 = QPointF(srcPos.x() + dist, srcPos.y())
        c2 = QPointF(dstPos.x() - dist, dstPos.y())
        p.cubicTo(c1, c2, dstPos)


def moveToFront(item):
    highest = -10000000000
    for item in item.scene().items():
        highest = max(highest, item.zValue())
    item.setZValue(highest + 1)


############################################################################################
############################################################################################
############################################################################################

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

    def __onPlugConnected(self, outPlug, inPlug):
        self.plugConnected.emit(outPlug, inPlug)


class PlugItem(QGraphicsItem):
    def __init__(self, opItem, plug):
        super(PlugItem, self).__init__()
        self.__plug = plug
        self.__opItem = opItem
        self.__label = QGraphicsTextItem(self)
        self.__pin = PinItem(self)
        self.setParentItem(self.__opItem)
        self.setName(plug.name())

    def setName(self, name):
        self.__label.setPlainText(name)
        self.layout()

    def boundingRect(self):
        return self.childrenBoundingRect()

    def paint(self, painter, option, widget=None):
        pass

    def pin(self):
        return self.__pin

    def plug(self):
        """
        @rtype: nap.Plug
        """
        return self.__plug

    def isInput(self):
        return isinstance(self.__plug, nap.InputPlugBase)

    def setUsable(self, b):
        self.setVisible(b)

    def layout(self):
        # textOffset = 6

        pinY = self.__label.boundingRect().height() / 2 - self.__pin.boundingRect().height() / 2

        if self.isInput():
            self.__label.setPos(self.pin().boundingRect().width(), 0)
            self.__pin.setPos(0, pinY)
        else:
            self.__label.setPos(0, 0)
            self.__pin.setPos(self.__label.boundingRect().width(), pinY)

        self.__opItem.layout()


class PinItem(QGraphicsPathItem):
    """ A Pinitem is a connector without the label """

    def __init__(self, plugItem):
        """
        @type plugItem: PlugItem
        """
        super(PinItem, self).__init__(plugItem)
        self.__plugItem = plugItem

        self.__color = self.plugItem().plug().core().typeColor(self.plugItem().plug().dataType())

        self.setPen(QPen(Qt.NoPen))
        self.setBrush(self.__color)

        p = QPainterPath()
        p.addRect(0, 0, 10, 10)
        self.setPath(p)

    def attachPos(self):
        r = self.boundingRect()
        if self.__plugItem.isInput():
            return QPointF(self.scenePos().x() - r.left(),
                           self.scenePos().y() + r.height() / 2)
        return QPointF(self.scenePos().x() + r.right(),
                       self.scenePos().y() + r.height() / 2)

    def plugItem(self):
        return self.__plugItem

    def color(self):
        return self.__color


class LayerItem(QGraphicsItem):
    def __init__(self):
        super(LayerItem, self).__init__()

    def boundingRect(self):
        return self.childrenBoundingRect()

    def paint(self, painter, option, widget=None):
        pass


class WireItem(QGraphicsPathItem):
    def __init__(self, srcPinItem, dstPinItem):
        """
        @type srcPinItem: PinItem
        @type dstPinItem: PinItem
        """
        super(WireItem, self).__init__()
        self.srcPin = srcPinItem
        self.dstPin = dstPinItem
        self.srcPos = QPointF()
        self.dstPos = QPointF()
        self.setFlag(QGraphicsItem.ItemIsFocusable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)


    def paint(self, painter, option, widget=None):
        if self.isVisible():
            self.updatePath()
        super(WireItem, self).paint(painter, option, widget)

    def updatePath(self):
        if self.srcPin:
            self.srcPos = self.srcPin.attachPos()
        if self.dstPin:
            self.dstPos = self.dstPin.attachPos()

        p = QPainterPath()
        calculateWirePath(self.srcPos, self.dstPos, p)
        self.setPath(p)

        pen = QPen()
        pen.setColor(self.srcPin.color())
        pen.setWidth(1)
        self.setPen(pen)


class WirePreview(QGraphicsPathItem):
    def __init__(self, parent):
        super(WirePreview, self).__init__(parent)
        self.srcPos = QPointF()
        self.dstPos = QPointF()
        self.srcPin = None
        self.dstPin = None
        self.setAcceptTouchEvents(False)
        self.setAcceptHoverEvents(False)

    def boundingRect(self):
        return self.path().boundingRect()

    def paint(self, painter, option, widget=None):
        if self.isVisible():
            self.updatePath()
            painter.setPen(self.pen())
            painter.drawPath(self.path())

    def updatePath(self):
        if self.srcPin:
            self.srcPos = self.srcPin.attachPos()
        if self.dstPin:
            self.dstPos = self.dstPin.attachPos()

        p = QPainterPath()
        calculateWirePath(self.srcPos, self.dstPos, p)
        self.setPath(p)

        pen = QPen()
        pen.setColor(COL_NODE_CONNECTION)
        pen.setWidth(1)
        pen.setStyle(Qt.DashLine)
        self.setPen(pen)


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

        self.__wireLayer = LayerItem()
        self.addItem(self.__wireLayer)
        self.__operatorLayer = LayerItem()
        self.addItem(self.__operatorLayer)
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

    def stopDragConnection(self, plugItem):
        if not self.__isWiring:
            return
        if plugItem:
            if self.__wireIsOutput:
                srcPlugItem = self.__previewWire.srcPin.plugItem()
                dstPlugItem = plugItem
            else:
                srcPlugItem = plugItem
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


    def dragConnectionSource(self):
        if self.__previewWire.srcPin:
            return self.__previewWire.srcPin.plugItem().plug()
        return self.__previewWire.dstPin.plugItem().plug()

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

    def __onOperatorAdded(self, op):
        """
        @type op: nap.Operator
        """
        item = OperatorItem(self.__operatorLayer, op)
        item.moved.connect(self.__updateSceneRect)
        item.setParentItem(self.__operatorLayer)
        item.plugConnected.connect(self.__addWire)
        item.setPos(_getObjectEditorPos(op))
        self.__updateSceneRect()


    def __onOperatorRemoved(self, op):
        self.__removeOperatorItem(self.findOperatorItem(op))

    def __onOperatorChanged(self, op):
        self.findOperatorItem(op).setPos(_getObjectEditorPos(op))

    def __removeOperatorItem(self, item):
        del item

    def __updateSceneRect(self):
        self.setSceneRect(
            self.itemsBoundingRect().adjusted(-1000, -1000, 1000, 1000))


class InteractMode(object):
    """ Base interaction mode, based on what happens, the mode may change to allow for more complex behaviour """

    def __init__(self):
        pass

    def mousePressed(self, view, evt):
        """
        @param view: PatchView
        @param evt: QMousePressEvent
        """
        raise NotImplementedError()

    def mouseMoved(self, view, evt):
        """
        @param view: PatchView
        @param evt: QMouseMoveEvent
        """
        raise NotImplementedError()

    def mouseReleased(self, view, evt):
        """
        @param view: PatchView
        @param evt: QMouseReleaseEvent
        """
        raise NotImplementedError()


class DefaultInteractMode(InteractMode):
    """ The view is ready for selection or any other interaction, this is the root interaction mode """

    def __init__(self):
        super(DefaultInteractMode, self).__init__()

    def init(self, view):
        view.viewport().unsetCursor()
        view.setDragMode(QGraphicsView.RubberBandDrag)

    def mousePressed(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        item = view.itemAt(evt.pos())
        if item:
            item.setVisible(True)
        pin = view.pinAt(evt.pos())
        opItem = view.operatorAt(evt.pos())

        if evt.buttons() == Qt.LeftButton:
            if pin:
                return True

            if opItem:
                view.setInteractMode(DragInteractMode)
                return False
        elif evt.buttons() == Qt.MiddleButton:
            view.setTransformationAnchor(QGraphicsView.NoAnchor)
            view.setInteractMode(PanInteractMode)
            return True

        elif evt.buttons() == Qt.RightButton:
            return True

        return False

    def mouseMoved(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        pin = view.pinAt(evt.pos())
        if pin:
            view.viewport().setCursor(Qt.PointingHandCursor)
        else:
            view.viewport().unsetCursor()
        return False

    def mouseReleased(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        if not view.scene():
            return True
        pin = view.pinAt(evt.pos())
        if pin:
            view.scene().startDragConnection(pin)
            view.setInteractMode(ConnectInteractMode)
            return True
        return False




class PatchView(QGraphicsView):
    """ The view the user will interact with when editing patches.

     Most likely this will be the only view for a patch,
     so it will contain one PatchScene and provide a set of InteractionModes to facilitate dragging items,
     panning and zooming the view and connecting Operators.
     """

    def __init__(self, ctx):
        """
        @param ctx: The application context
        @type ctx: AppContext
        """
        self.ctx = ctx
        super(PatchView, self).__init__()
        self.setRenderHint(QPainter.Antialiasing, True)
        self.__interactMode = None
        self.__oldMousePos = QPointF()
        self.__mouseClickPos = QPointF()
        self.setInteractMode(DefaultInteractMode)
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(
            self.__onCustomContextMenuRequested)

        deleteAction = QAction('Delete Selected', self)
        deleteAction.setShortcut(QKeySequence.Delete)
        deleteAction.triggered.connect(self.__onDeleteSelected)
        self.addAction(deleteAction)

    def __onDeleteSelected(self):
        assert False
        for item in self.scene().selectedItems():
            print(item)

    def focusNextChild(self, next):
        return False

    def setInteractMode(self, mode):
        self.__interactMode = mode()
        self.__interactMode.init(self)

    def __onCustomContextMenuRequested(self, pos):
        if not self.scene():
            return
        clickedItems = self.items(pos)

        menu = QMenu(self)

        if not clickedItems:
            patch = self.scene().patch()
            addCompMenu = menu.addMenu(iconstore.icon('brick_add'),
                                       'Add Operator...')
            self.ctx.createObjectActions(patch, self.ctx.core().operatorTypes(),
                                         addCompMenu)

        menu.exec_(QCursor.pos())


    def mousePressEvent(self, evt):
        if not self.__interactMode.mousePressed(self, evt):
            super(PatchView, self).mousePressEvent(evt)

    def mouseMoveEvent(self, evt):
        if not self.__interactMode.mouseMoved(self, evt):
            super(PatchView, self).mouseMoveEvent(evt)

    def mouseReleaseEvent(self, evt):
        if not self.__interactMode.mouseReleased(self, evt):
            super(PatchView, self).mouseReleaseEvent(evt)

    def wheelEvent(self, evt):
        return
        self.setTransformationAnchor(QGraphicsView.AnchorUnderMouse)

        scaleFactor = 1.15
        if evt.angleDelta().y() > 0:
            self.scale(scaleFactor, scaleFactor)
        else:
            self.scale(1 / scaleFactor, 1 / scaleFactor)

    def resetZoom(self):
        dx = self.transform().dx()
        dy = self.transform().dy()
        self.setTransform(QTransform.fromTranslate(dx, dy))

    def _itemAt(self, scenePos, itemType):
        for item in self.items(scenePos):
            if isinstance(item, itemType):
                return item

    def pinAt(self, scenePos):
        """
        @rtype: PinItem
        """
        return self._itemAt(scenePos, PinItem)

    def plugAt(self, scenePos):
        """
        @rtype: PlugItem
        """
        return self._itemAt(scenePos, PlugItem)

    def operatorAt(self, scenePos):
        """
        @rtype: OperatorItem
        """
        return self._itemAt(scenePos, OperatorItem)


class PatchEditor(QWidget):
    """ The main widget holding a patch view amongst others """

    def __init__(self, ctx):
        """
        @type ctx: AppContext
        """
        self.ctx = ctx
        super(PatchEditor, self).__init__()
        self.setLayout(QHBoxLayout())
        self.__patchView = PatchView(ctx)
        self.layout().addWidget(self.__patchView)
        # self.__scene = PatchScene()
        # self.__patchView.setScene(self.__scene)

    def setModel(self, patch):
        if isinstance(patch, nap.Component):
            patch = patch.childOftype('nap::Patch')
        self.__patchView.setScene(PatchScene(self.ctx, patch))


class DragInteractMode(InteractMode):
    """ When the user drags items around (not the rubberband) """

    def __init__(self):
        super(DragInteractMode, self).__init__()
        self.__oldMousePos = QPointF()
        self.__mouseClickPos = QPointF()

    def init(self, view):
        """
        @type view: PatchView
        """
        self.__oldMousePos = view.mapFromGlobal(QCursor.pos())
        self.__mouseClickPos = self.__oldMousePos

    def mousePressed(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        self.__mouseClickPos = evt.pos()
        return False

    def mouseMoved(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        pos = evt.pos()
        delta = view.mapToScene(pos) - view.mapToScene(self.__oldMousePos)
        for opItem in view.scene().selectedOperatorItems():
            opItem.moveBy(delta.x(), delta.y())
        self.__oldMousePos = pos
        return False

    def mouseReleased(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        if not view.scene():
            return True
        delta = evt.pos() - self.__mouseClickPos
        if not delta.isNull():
            for opItem in view.scene().selectedOperatorItems():
                _setObjectEditorPos(opItem.operator(), opItem.pos())
        view.setInteractMode(DefaultInteractMode)
        return False


class PanInteractMode(InteractMode):
    """ When the user pans or zooms the view """

    def __init__(self):
        super(PanInteractMode, self).__init__()
        self.__oldMousePos = QPointF()

    def init(self, view):
        """
        @type view: PatchView
        """
        view.viewport().setCursor(Qt.ClosedHandCursor)
        view.setDragMode(QGraphicsView.NoDrag)
        self.__oldMousePos = view.mapFromGlobal(QCursor.pos())

    def mousePressed(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        self.__oldMousePos = QPointF()
        return False

    def mouseMoved(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        delta = evt.pos() - self.__oldMousePos
        view.horizontalScrollBar().setValue(
            view.horizontalScrollBar().value() - delta.x())
        view.verticalScrollBar().setValue(
            view.verticalScrollBar().value() - delta.y())
        self.__oldMousePos = evt.pos()
        return False

    def mouseReleased(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        if not view.scene():
            return True
        view.setInteractMode(DefaultInteractMode)
        return False


class ConnectInteractMode(InteractMode):
    def __init__(self):
        super(ConnectInteractMode, self).__init__()
        self.__oldMousePos = QPointF()

    def init(self, view):
        """
        @type view: PatchView
        """
        view.setDragMode(QGraphicsView.NoDrag)

    def mousePressed(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        plug = view.plugAt(evt.pos())
        print("stopDrag")
        view.scene().stopDragConnection(plug)
        return False

    def mouseMoved(self, view:PatchView, evt:QMouseEvent):
        pin = view.pinAt(evt.pos())
        srcPlug = view.scene().dragConnectionSource()
        pt = view.mapToScene(evt.pos())
        if pin:
            plug = pin.plugItem()
            if plug and plug.plug() and _canConnect(srcPlug, plug.plug()):
                pt = plug.pin().attachPos()
                view.viewport().setCursor(Qt.PointingHandCursor)
            else:
                view.viewport().unsetCursor()
        view.scene().updateDragConnection(pt)

        self.__oldMousePos = evt.pos()
        return False

    def mouseReleased(self, view, evt):
        """
        @type view: PatchView
        @type evt: QMouseEvent
        """
        if not view.scene():
            return True
        view.setInteractMode(DefaultInteractMode)
        return False
