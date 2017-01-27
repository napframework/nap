import sys

from appcontext import AppContext
from coresettings.corewidget import ConnectionWidget
from log.logpanel import LogPanel
from outline.model import *
from outline.outlinewidget import OutlineWidget
from patch.patcheditor import PatchEditor

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
        # self.setAnimated(False)
        self.setStatusBar(QStatusBar())
        self.__root = None
        self.__editors = {}  # path:qwidget

        self.ctx.core().rootChanged.connect(self.__onRootChanged)
        self.ctx.core().objectRemoved.connect(self.__onObjectRemoved)
        self.ctx.connectionChanged.connect(self.__onConnectionChanged)
        self.ctx.selectionChanged.connect(self.__onSelectionChanged)
        self.ctx.editorRequested.connect(self.__onEditorRequested)
        self.ctx.logMessageReceived.connect(self.__onLogMessage)
        self.ctx.appSuspended.connect(self.__onSuspended)
        self.ctx.appResumed.connect(self.__onResumed)

        self.__setupUi()
        self.__restore()

        fileMenu = self.menuBar().addMenu('File')
        openFileAction = QAction('Open...', fileMenu)
        openFileAction.setShortcut(QKeySequence.Open)
        openFileAction.triggered.connect(self.__onLoadFile)
        fileMenu.addAction(openFileAction)

        # connect to host saved in settings
        s = QSettings()
        host = str(s.value(LAST_HOST, "tcp://localhost:8888"))
        self.ctx.connect(host)

    def __onSuspended(self):
        self.setEnabled(False)

    def __onResumed(self):
        self.setEnabled(True)

    def __onLoadFile(self, enabled):
        filename = QFileDialog.getOpenFileName(self, 'Select file to load',
                                               self.ctx.lastFileDir(),
                                               self.ctx.napFileFilter())
        filename = filename[0]  # First was filename, second is filter
        if not filename:
            return

        self.ctx.loadFile(filename)

    def __onLogMessage(self, level, levelName, text):
        self.statusBar().showMessage(text)

    def __onObjectRemoved(self, obj):
        editor = self.__getEditor(obj)
        if editor:
            self.destroyDock(editor)

    def __onEditorRequested(self, obj):
        path = obj.pathString()
        print('Editor for: %s: %s' % (path, self.ctx.editorTypeFor(obj)))
        self.__getOrCreateEditor(obj)

    def __getEditor(self, obj):
        if not isinstance(obj, str):
            obj = obj.pathString()
        if not obj in self.__editors.keys():
            return None
        return self.__editors[obj]

    def __getOrCreateEditor(self, obj):
        if not isinstance(obj, str):
            objPath = obj.pathString()
        editorType = self.ctx.editorTypeFor(obj)
        title = '%s (%s)' % (editorType.__name__, objPath)

        editor = self.__getEditor(obj)
        if not editor:
            editor = editorType(self.ctx)
            editor.setModel(obj)
            self.__editors[objPath] = self.addDock(editor, Qt.TopDockWidgetArea,
                                                   title)

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
        self.ctx.core()._handle_getObjectTree()

    def __onRootChanged(self):
        """
        @type obj: nap.Object
        """
        self.outline.setRoot(self.ctx.core().root())
        self.__restoreEditors()

    def __onSelectionChanged(self, obj):
        if obj and isinstance(obj, list):
            obj = obj[0]
        self.inspector.setRoot(obj)
        self.inspector.expandAll()

    def __setupUi(self):
        self.outline = OutlineWidget(self.ctx, 'outline')
        self.outline.setPropagateSelection(True)
        self.outline.setFilterTypes([nap.Entity])
        self.addDock(self.outline, Qt.LeftDockWidgetArea, 'Outline')

        self.connector = ConnectionWidget(self.ctx)
        self.addDock(self.connector, Qt.TopDockWidgetArea, 'Core')

        self.inspector = OutlineWidget(self.ctx, 'inspector')
        self.inspector.setFilterTypes([nap.Component, nap.Attribute])
        self.inspector.setRootVisible(False)
        self.addDock(self.inspector, Qt.RightDockWidgetArea, 'Attributes')

        self.logger = LogPanel(self.ctx)
        self.addDock(self.logger, Qt.BottomDockWidgetArea, 'Log')

    def __restore(self):
        s = QSettings()
        geo = s.value(WIN_GEO)
        if geo:
            self.restoreGeometry(geo)
        state = s.value(WIN_STATE)
        if state:
            self.restoreState(state)

    def __saveState(self):
        s = QSettings()
        s.setValue(WIN_GEO, self.saveGeometry())
        s.setValue(WIN_STATE, self.saveState())

        editedObjects = self.editedObjects()
        print('Storing edited: %s' % editedObjects)
        s.setValue(EDITED_OBJECTS, editedObjects)

    def onConnected(self):
        self.__root = Object.root()

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
        self.__saveState()
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
