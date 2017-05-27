from PyQt5.QtCore import Qt, QPointF
from PyQt5.QtGui import QCursor, QMouseEvent, QPainter, QKeySequence, QTransform
from PyQt5.QtWidgets import QGraphicsView, QAction, QMenu

import iconstore
from actions import DisconnectPlugsAction, RemoveObjectsAction
from patch.layeritem import LayerItem
from patch.operatoritem import OperatorItem

from patch.patchutils import _setObjectEditorPos, _canConnect
from patch.pinitem import PinItem
from patch.plugitem import PlugItem
from patch.wireitem import WireItem
from utils.butils import _excludeTypes, _filter


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
        filteredClickedItems = _excludeTypes(clickedItems, [LayerItem])

        menu = QMenu(self)

        if not filteredClickedItems:
            patch = self.scene().patch()
            addCompMenu = menu.addMenu(iconstore.icon('brick_add'),
                                       'Add Operator...')
            self.ctx.createObjectActions(patch, self.ctx.core().operatorTypes(),
                                         addCompMenu)
        else:
            wires = list(_filter(clickedItems, WireItem))
            if len(wires) > 0:
                plugs = []
                for wire in wires:
                    plugs.append(wire.dstPin.plugItem().plug())
                a = DisconnectPlugsAction(self.ctx, plugs)
                a.setParent(menu)
                menu.addAction(a)

            operatorItems = list(_filter(clickedItems, OperatorItem))
            if len(operatorItems) > 0:
                operators = []
                for item in operatorItems:
                    operators.append(item.operator())
                a = RemoveObjectsAction(self.ctx, operators)
                a.setParent(menu)
                menu.addAction(a)

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
        @rtype: patch.pinitem.PinItem
        """
        return self._itemAt(scenePos, PinItem)

    def plugAt(self, scenePos):
        """
        @rtype: patch.plugitem.PlugItem
        """
        return self._itemAt(scenePos, PlugItem)

    def operatorAt(self, scenePos):
        """
        @rtype: OperatorItem
        """
        return self._itemAt(scenePos, OperatorItem)


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
        pin = view.pinAt(evt.pos())
        print("stopDrag")
        view.scene().stopDragConnection(pin)
        return False

    def mouseMoved(self, view: PatchView, evt: QMouseEvent):
        pin = view.pinAt(evt.pos())
        srcPlug = view.scene().dragConnectionSource()
        pt = view.mapToScene(evt.pos())
        if pin and srcPlug:
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


