import os

from PyQt5.QtCore import *

import core_native.nap
import core_py.nap
import nap

CORE_TYPES = [
    core_py.nap.Core, core_native.nap.Core
]


class AppContext(QObject):
    coreChanged = pyqtSignal(object)
    selectionChanged = pyqtSignal(object)
    applicationClosing = pyqtSignal()
    logMessageReceived = pyqtSignal(int, str, str)
    appSuspended = pyqtSignal()
    appResumed = pyqtSignal()

    editorRequested = pyqtSignal(object)

    __NAP_FILE_FILTER = ('NAP File', 'json')
    __LAST_OPENED_FILE = 'lastOpenedFile'

    def __init__(self):
        super(AppContext, self).__init__()
        self.__core = core_py.nap.Core()
        self.__selectedObjects = None
        self.__editorTypes = {}
        self.__editors = {}
        self.__suspended = False
        self.__core.logMessageReceived.connect(self.logMessageReceived)
        self.__core.messageReceived.connect(self.__onMessageReceived)
        self.__core.waitingForMessage.connect(self.__onWaitingFormessage)

    def __onWaitingFormessage(self):
        self.suspendApp()

    def __onMessageReceived(self, *args):
        self.resumeApp()

    def registerEditor(self, objType, editorType):
        self.__editorTypes[objType] = editorType

    def core(self) -> nap.Core:
        return self.__core

    def setCore(self, core=None):
        if not core:
            core = CORE_TYPES[0]()
        self.__core = core
        self.coreChanged.emit(core)
        self.__core.refresh()

    def hasEditorFor(self, obj):
        return obj.typename() in self.__editorTypes.keys()

    def editorTypeFor(self, obj):
        if not self.hasEditorFor(obj):
            return None
        return self.__editorTypes[obj.typename()]

    def requestEditorFor(self, obj):
        self.editorRequested.emit(obj)

    def selection(self):
        return self.__selectedObjects

    def setSelection(self, objects):
        self.__selectedObjects = objects
        self.selectionChanged.emit(objects)

    def iconStore(self):
        return self.__iconStore

    def createObjectActions(self, parentObj, typeList, menu):
        for objType in typeList:
            a = AddChildAction(self, parentObj, objType)
            a.setParent(menu)
            menu.addAction(a)
            # yield action

    def napFileExtension(self):
        return self.__NAP_FILE_FILTER[1]

    def napFileFilter(self):
        return '%s (*.%s)' % self.__NAP_FILE_FILTER

    def napFileDescription(self):
        return self.__NAP_FILE_FILTER[0]

    def ensureNAPFileExtension(self, filename):
        ext = self.napFileExtension()
        if not filename.lower().endswith('.%s' % ext):
            return '%s.%s' % (filename, ext)
        return filename

    def suspendApp(self):
        if not self.__suspended:
            self.__suspended = True
            self.appSuspended.emit()

    def resumeApp(self):
        if self.__suspended:
            self.__suspended = False
            self.appResumed.emit()

    def lastFileDir(self):
        s = QSettings()
        lastFilename = s.value(self.__LAST_OPENED_FILE)
        if lastFilename:
            return os.path.dirname(str(lastFilename))

    def __setLastFilename(self, filename):
        s = QSettings()
        s.setValue(self.__LAST_OPENED_FILE, filename)

    def importObject(self, parentObj, filename):
        self.core().importObject(parentObj, filename)
        self.__setLastFilename(filename)

    def exportObject(self, obj, filename):
        filename = self.ensureNAPFileExtension(filename)
        self.core().exportObject(obj, filename)
        self.__setLastFilename(filename)

    def loadFile(self, filename):
        self.core().loadFile(filename)
        self.__setLastFilename(filename)
