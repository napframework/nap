from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from patch.nodeitem import NodeItem, SocketItem, PinItem


class GraphView(QGraphicsView):
    """ The view the user will interact with when editing patches.

     Most likely this will be the only view for a patch,
     so it will contain one PatchScene and provide a set of InteractionModes to facilitate dragging items,
     panning and zooming the view and connecting Operators.
     """

    def __init__(self):
        """
        @param ctx: The application context
        @type ctx: AppContext
        """
        super(GraphView, self).__init__()
        self.setRenderHint(QPainter.Antialiasing, True)
        self.__interactMode = None
        self.__oldMousePos = QPointF()
        self.__mouseClickPos = QPointF()
        self.setInteractMode(DefaultInteractMode)
        self.setContextMenuPolicy(Qt.CustomContextMenu)

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

    def mousePressEvent(self, evt):
        if not self.__interactMode.mousePressed(self, evt):
            super(GraphView, self).mousePressEvent(evt)

    def mouseMoveEvent(self, evt):
        if not self.__interactMode.mouseMoved(self, evt):
            super(GraphView, self).mouseMoveEvent(evt)

    def mouseReleaseEvent(self, evt):
        if not self.__interactMode.mouseReleased(self, evt):
            super(GraphView, self).mouseReleaseEvent(evt)

    def wheelEvent(self, evt):
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

    def socketAt(self, scenePos) -> SocketItem:
        for item in self.items(scenePos):
            if isinstance(item, NodeItem):
                return None  # keep from selecting items behind nodes
            if isinstance(item, SocketItem):
                return item

    def pinAt(self, scenePos) -> PinItem:
        for item in self.items(scenePos):
            if isinstance(item, NodeItem):
                return None  # keep from selecting items behind nodes
            if isinstance(item, PinItem):
                return item

    def operatorAt(self, scenePos) -> NodeItem:
        return self._itemAt(scenePos, NodeItem)


class InteractMode(object):
    """ Base interaction mode, based on what happens, the mode may change to allow for more complex behaviour """

    def __init__(self):
        pass

    def mousePressed(self, view: GraphView, evt: QMouseEvent):
        raise NotImplementedError()

    def mouseMoved(self, view: GraphView, evt: QMouseEvent):
        raise NotImplementedError()

    def mouseReleased(self, view: GraphView, evt: QMouseEvent):
        raise NotImplementedError()


class DefaultInteractMode(InteractMode):
    """ The view is ready for selection or any other interaction, this is the root interaction mode """

    def __init__(self):
        super(DefaultInteractMode, self).__init__()

    def init(self, view):
        view.viewport().unsetCursor()
        view.setDragMode(QGraphicsView.RubberBandDrag)

    def mousePressed(self, view: GraphView, evt: QMouseEvent):
        pin = view.pinAt(evt.pos())
        node = view.operatorAt(evt.pos())

        if evt.buttons() == Qt.LeftButton:
            if pin:
                return True

            if node:
                view.setInteractMode(DragInteractMode)
                return False
        elif evt.buttons() == Qt.MiddleButton:
            view.setTransformationAnchor(QGraphicsView.NoAnchor)
            view.setInteractMode(PanInteractMode)
            return True

        elif evt.buttons() == Qt.RightButton:
            return True

        return False

    def mouseMoved(self, view: GraphView, evt: QMouseEvent):
        pin = view.pinAt(evt.pos())
        if pin:
            view.viewport().setCursor(Qt.PointingHandCursor)
        else:
            view.viewport().unsetCursor()
        return False

    def mouseReleased(self, view: GraphView, evt: QMouseEvent):
        if not view.scene():
            return True
        pin = view.pinAt(evt.pos())
        if pin:
            view.scene().startDragConnection(pin.socket())
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
        @type view: GraphView
        """
        self.__oldMousePos = view.mapFromGlobal(QCursor.pos())
        self.__mouseClickPos = self.__oldMousePos

    def mousePressed(self, view: GraphView, evt: QMouseEvent):
        self.__mouseClickPos = evt.pos()
        return False

    def mouseMoved(self, view: GraphView, evt: QMouseEvent):
        pos = evt.pos()
        delta = view.mapToScene(pos) - view.mapToScene(self.__oldMousePos)
        for opItem in view.scene().selectedNodes():
            opItem.moveBy(delta.x(), delta.y())
        self.__oldMousePos = pos
        return False

    def mouseReleased(self, view: GraphView, evt: QMouseEvent):
        if not view.scene():
            return True
        # delta = evt.pos() - self.__mouseClickPos
        # if not delta.isNull():
        #     for opItem in view.scene().selectedOperatorItems():
        #         _setObjectEditorPos(opItem.operator(), opItem.pos())
        view.setInteractMode(DefaultInteractMode)
        return False


class PanInteractMode(InteractMode):
    """ When the user pans or zooms the view """

    def __init__(self):
        super(PanInteractMode, self).__init__()
        self.__oldMousePos = QPointF()

    def init(self, view: GraphView):
        view.viewport().setCursor(Qt.ClosedHandCursor)
        view.setDragMode(QGraphicsView.NoDrag)
        self.__oldMousePos = view.mapFromGlobal(QCursor.pos())

    def mousePressed(self, view: GraphView, evt: QMouseEvent):
        self.__oldMousePos = QPointF()
        return False

    def mouseMoved(self, view: GraphView, evt: QMouseEvent):
        delta = evt.pos() - self.__oldMousePos
        view.horizontalScrollBar().setValue(
            view.horizontalScrollBar().value() - delta.x())
        view.verticalScrollBar().setValue(
            view.verticalScrollBar().value() - delta.y())
        self.__oldMousePos = evt.pos()
        return False

    def mouseReleased(self, view: GraphView, evt: QMouseEvent):
        if not view.scene():
            return True
        view.setInteractMode(DefaultInteractMode)
        return False


class ConnectInteractMode(InteractMode):
    def __init__(self):
        super(ConnectInteractMode, self).__init__()
        self.__oldMousePos = QPointF()

    def init(self, view: GraphView):
        view.setDragMode(QGraphicsView.NoDrag)

    def mousePressed(self, view: GraphView, evt: QMouseEvent):
        socket = view.socketAt(evt.pos())
        view.scene().stopDragConnection(socket)
        return False

    def mouseMoved(self, view: GraphView, evt: QMouseEvent):
        srcSocket = view.scene().dragConnectionSource()
        dstSocket = view.socketAt(evt.pos())

        # pos, vec = srcSocket.attachPosVec()
        vec = QPointF()
        pos = view.mapToScene(evt.pos())

        if dstSocket:
            if view.scene().canConnect(srcSocket, dstSocket):
                pos, vec = dstSocket.attachPosVec()
                view.viewport().setCursor(Qt.PointingHandCursor)
            else:
                view.viewport().unsetCursor()

        view.scene().updateDragConnection(pos, vec)

        self.__oldMousePos = evt.pos()
        return False

    def mouseReleased(self, view: GraphView, evt: QMouseEvent):
        if not view.scene():
            return True
        view.setInteractMode(DefaultInteractMode)
        return False
