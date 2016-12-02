from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import iconstore
import nap

class AddChildAction(QAction):
    def __init__(self, ctx, parentObj, typename):
        """
        @type ctx: nap.Core
        """
        super(AddChildAction, self).__init__(iconstore.icon('brick_add'), typename, None)
        self.__parentObj = parentObj
        self.__ctx = ctx
        self.__typename = typename
        self.triggered.connect(self.perform)

    def perform(self, b):
        self.__ctx.core().addChild(self.__parentObj, self.__typename)



class AppContext(QObject):
    connectionChanged = pyqtSignal(bool, str, str)
    selectionChanged = pyqtSignal(object)
    applicationClosing = pyqtSignal()
    logMessageReceived = pyqtSignal(int, str, str)

    editorRequested = pyqtSignal(object)

    def __init__(self):
        super(AppContext, self).__init__()
        self.__core = nap.Core()
        self.__selectedObjects = None
        self.__editorTypes = {}
        self.__editors = {}
        self.connectionChanged.connect(self.__onConnectionChanged)
        self.__core.logMessageReceived.connect(self.logMessageReceived)

    def registerEditor(self, objType, editorType):
        self.__editorTypes[objType] = editorType

    def core(self):
        """
        @rtype: nap.Core
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
        # self.__core.objectTree()

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
