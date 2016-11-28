from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import iconstore
import napclient


class AppContext(QObject):
    connectionChanged = pyqtSignal(bool, str, str)
    selectionChanged = pyqtSignal(object)
    applicationClosing = pyqtSignal()

    editorRequested = pyqtSignal(object)

    def __init__(self):
        super(AppContext, self).__init__()
        self.__core = napclient.Core()
        self.__selectedObject = None
        self.__editorTypes = {}
        self.__editors = {}
        self.connectionChanged.connect(self.__onConnectionChanged)

    def registerEditor(self, objType, editorType):
        self.__editorTypes[objType] = editorType

    def core(self):
        """
        @rtype: napclient.Core
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
        self.core().onGetObjectTree()

    def connect(self, host):
        self.__core.getModuleInfo()
        # self.__core.objectTree()

    def setSelection(self, obj):
        self.__selectedObject = obj
        self.selectionChanged.emit(obj)

    def iconStore(self):
        return self.__iconStore

    def createObjectActions(self, parentObj, typeList, menu):
        for objType in typeList:
            action = QAction(iconstore.icon('brick_add'), objType, menu)
            action.triggered[(bool)].connect(lambda objType=objType: self.core().addChild(parentObj, objType))
            menu.addAction(action)
            # yield action
