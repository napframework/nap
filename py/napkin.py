import sys

from appcontext import AppContext
from connectionwidget import ConnectionWidget
from model import *
from napclient import *
from outline import OutlineWidget
from patchpanel import PatchEditor

WIN_GEO = 'WindowGeometry'
WIN_STATE = 'WindowState'
EDITED_OBJECTS = 'EditorList'
LAST_HOST = 'tcp://localhost:8888'


class MainWindow(QMainWindow):
    def __init__(self, ctx):
        """
        @type ctx: AppContext
        """
        super(MainWindow, self).__init__()
        self.ctx = ctx
        self.setWindowTitle(QCoreApplication.applicationName())
        self.setDockNestingEnabled(True)
        self.__root = None
        self.__editors = {}  # path:qwidget

        self.ctx.core().rootChanged.connect(self.__onRootChanged)
        self.ctx.core().objectRemoved.connect(self.__onObjectRemoved)
        self.ctx.connectionChanged.connect(self.__onConnectionChanged)
        self.ctx.selectionChanged.connect(self.__onSelectionChanged)
        self.ctx.editorRequested.connect(self.__onEditorRequested)

        self.__setupUi()
        self.__restore()

        s = QSettings()
        host = str(s.value(LAST_HOST, "tcp://localhost:8888"))
        self.ctx.connect(host)

    def __onObjectRemoved(self, obj):
        editor = self.__getEditor(obj)
        if editor:
            self.destroyDock(editor)

    def __onEditorRequested(self, obj):
        path = obj.pathString()
        print('Editor for: %s: %s' % (path, self.ctx.editorTypeFor(obj)))
        self.__getOrCreateEditor(obj)

    def __getEditor(self, obj):
        if not isinstance(obj, basestring):
            obj = obj.pathString()
        if not obj in self.__editors.keys():
            return None
        return self.__editors[obj]

    def __getOrCreateEditor(self, obj):
        if not isinstance(obj, basestring):
            objPath = obj.pathString()
        editorType = self.ctx.editorTypeFor(obj)
        title = '%s (%s)' % (editorType.__name__, objPath)

        editor = self.__getEditor(obj)
        if not editor:
            editor = editorType(self.ctx)
            editor.setModel(obj)
            self.__editors[objPath] = self.addDock(editor, Qt.TopDockWidgetArea, title)

        else:
            editor.show()

    def __restoreEditors(self):
        s = QSettings()
        editedObjects = s.value(EDITED_OBJECTS)
        if editedObjects:
            for objPath in editedObjects:
                obj = self.ctx.core().resolvePath(str(objPath))
                if obj:
                    self.__getOrCreateEditor(obj)
                else:
                    print('Object not found: %s ' % objPath)
        self.__restore()

    def editedObjects(self):
        return list(self.__editors.keys())

    def __onConnectionChanged(self, *args):
        self.ctx.core().onGetObjectTree()

    def __onRootChanged(self):
        """
        @type obj: napclient.NObject
        """
        self.outline.setRoot(self.ctx.core().root())
        self.__restoreEditors()

    def __onSelectionChanged(self, obj):
        if obj and isinstance(obj, list):
            obj = obj[0]
        self.inspector.setRoot(obj)
        self.inspector.expandAll()

    def __setupUi(self):
        self.outline = OutlineWidget(self.ctx)
        self.outline.setPropagateSelection(True)
        self.outline.setFilterTypes([napclient.Entity])
        self.addDock(self.outline, Qt.LeftDockWidgetArea, 'Outline')

        self.connector = ConnectionWidget(self.ctx)
        self.addDock(self.connector, Qt.TopDockWidgetArea, 'Connection')

        self.inspector = OutlineWidget(self.ctx)
        self.inspector.setFilterTypes([napclient.Component, napclient.Attribute])
        self.inspector.setRootVisible(False)
        self.addDock(self.inspector, Qt.RightDockWidgetArea, 'Attributes')

    def __restore(self):
        s = QSettings()
        geo = s.value(WIN_GEO)
        if geo:
            self.restoreGeometry(geo)
        state = s.value(WIN_STATE)
        if state:
            self.restoreState(state)

    def onConnected(self):
        self.__root = NObject.root()

    def addDock(self, w, area, name):
        dock = QDockWidget(self)
        dock.setObjectName(name)
        dock.setWindowTitle(name)
        dock.setWidget(w)
        w.layout().setContentsMargins(2, 2, 2, 2)
        self.addDockWidget(area, dock)
        return dock

    def destroyDock(self, dock):
        # just in case we're dealing with an editor here
        for p in self.__editors.keys():
            if self.__editors[p] == dock:
                del self.__editors[p]
        self.removeDockWidget(dock)
        dock.destroy()

    def closeEvent(self, evt):
        self.ctx.applicationClosing.emit()
        s = QSettings()
        s.setValue(WIN_GEO, self.saveGeometry())
        s.setValue(WIN_STATE, self.saveState())

        editedObjects = self.editedObjects()
        print('Storing edited: %s' % editedObjects)

        s.setValue(EDITED_OBJECTS, editedObjects)
        QWidget.closeEvent(self, evt)


if __name__ == '__main__':
    QCoreApplication.setApplicationName('NapEditor')
    QCoreApplication.setOrganizationName('Naivi')

    app = QApplication(sys.argv)
    app.setAttribute(Qt.AA_DontShowIconsInMenus, False)

    ctx = AppContext()
    ctx.registerEditor('nap::PatchComponent', PatchEditor)

    win = MainWindow(ctx)
    win.show()

    app.exec_()
