import os

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

import core
import iconstore
import nap


class AddChildAction(QAction):
    def __init__(self, ctx, parentObj, typename):
        """
        @type ctx: core.Core
        """
        super(AddChildAction, self).__init__(iconstore.icon('brick_add'), typename, None)
        self.__parentObj = parentObj
        self.__ctx = ctx
        self.__typename = typename
        self.triggered.connect(self.perform)

    def perform(self, b):
        self.__ctx.core().addChild(self.__parentObj, self.__typename)


class RemoveObjectsAction(QAction):
    def __init__(self, ctx, objects):
        super(RemoveObjectsAction, self).__init__(iconstore.icon('delete'), 'Remove', None)
        self.__ctx = ctx
        self.__objects = objects
        self.triggered.connect(self.perform)

    def perform(self, b):
        self.__ctx.core().removeObjects(self.__objects)


class DisconnectPlugsAction(QAction):
    def __init__(self, ctx, plugs):
        """
        @type ctx: core.Core
        """
        super(DisconnectPlugsAction, self).__init__(iconstore.icon('delete'), 'Disconnect', None)
        self.__ctx = ctx
        self.__plugs = plugs
        self.triggered.connect(self.perform)

    def perform(self, b):
        for plug in self.__plugs:
            self.__ctx.core().disconnectPlug(plug)



class AppContext(QObject):
    connectionChanged = pyqtSignal(bool, str, str)
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
        self.__core = core.Core()
        self.__selectedObjects = None
        self.__editorTypes = {}
        self.__editors = {}
        self.__suspended = False
        self.connectionChanged.connect(self.__onConnectionChanged)
        self.__core.logMessageReceived.connect(self.logMessageReceived)
        self.__core.messageReceived.connect(self.__onMessageReceived)
        self.__core.waitingForMessage.connect(self.__onWaitingFormessage)


    def __onWaitingFormessage(self):
        self.suspendApp()

    def __onMessageReceived(self, *args):
        self.resumeApp()

    def registerEditor(self, objType, editorType):
        self.__editorTypes[objType] = editorType

    def core(self):
        """
        @rtype: core.Core
        """
        return self.__core

    def hasEditorFor(self, obj):
        return obj.typename() in self.__editorTypes.keys()

    def editorTypeFor(self, obj):
        if not self.hasEditorFor(obj):
            return None
        return self.__editorTypes[obj.typename()]

    def requestEditorFor(self, obj):
        self.editorRequested.emit(obj)

    def __onConnectionChanged(self):
        self.core()._handle_getObjectTree()

    def connect(self, host):
        self.__core.loadModuleInfo()

    def selection(self, types=None):
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
