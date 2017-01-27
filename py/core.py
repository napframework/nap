import json

from PyQt5.QtCore import QObject, pyqtSignal
from PyQt5.QtWidgets import QApplication

from asyncjsonclient import AsyncJSONClient
from nap import _J_NAME, _J_BASETYPES, _J_INSTANTIABLE, Object, _stripCPPNamespace, _allSubClasses, _J_VALUE_TYPE, \
    Attribute, _J_TYPE, Operator, Component, OutputPlugBase, InputPlugBase
from utils import qtutils


class Core(QObject):
    """ Core represents a NAP Application structure.
    """

    rootChanged = pyqtSignal()
    moduleInfoChanged = pyqtSignal(dict)
    typeHierarchyChanged = pyqtSignal()
    objectRemoved = pyqtSignal(object)
    logMessageReceived = pyqtSignal(int, str, str)
    messageReceived = pyqtSignal()
    waitingForMessage = pyqtSignal()

    def __init__(self, host: str = 'tcp://localhost:8888'):
        super(Core, self).__init__()
        self.__rpc = AsyncJSONClient(host)
        self.__rpc.messageReceived.connect(self.handleMessageReceived)
        self.__componentTypes = None
        self.__dataTypes = None
        self.__operatorTypes = None
        self.__root = None
        self.__objects = {}
        self.__types = []
        self.__metatypes = {}
        self.__typeColors = {}

    def __waitForMessage(self):
        self.waitingForMessage.emit()

    def resolvePath(self, path):
        return self.root().resolvePath(path)

    def setObject(self, ptr, obj):
        self.__objects[ptr] = obj

    def findObject(self, ptr: int):
        if not isinstance(ptr, int):
            ptr = int(ptr)
        return self.__objects[ptr]

    def types(self):
        return self.__types

    def typeIndex(self, typename):
        """ Return the index of the type
        TODO: Retrieve from server
        """
        if not typename:
            return 0
        i = 0
        for t in self.types():
            if t['name'] == typename:
                return i
            i += 1

    def baseTypes(self, typename):
        for t in self.__types:
            if t[_J_NAME] == typename:
                return t[_J_BASETYPES]

    def subTypes(self, baseTypename, instantiable=False):
        for t in self.__types:
            if not baseTypename in t[_J_BASETYPES]:
                continue
            if instantiable and not t[_J_INSTANTIABLE]:
                continue
            yield t[_J_NAME]

    def typeColor(self, typename):
        if typename in self.__typeColors:
            return self.__typeColors[typename]

        col = qtutils.randomColor(self.typeIndex(typename))
        self.__typeColors[typename] = col
        return col

    def __getOrCreateMetaType(self, cppTypename, clazz=None):
        if not clazz:
            clazz = Object
        pythonTypename = _stripCPPNamespace(cppTypename)

        if pythonTypename in self.__metatypes.keys():
            return self.__metatypes[pythonTypename]

        t = type(pythonTypename, (clazz,), dict())
        print('Created metatype: %s' % t)
        self.__metatypes[pythonTypename] = t
        return t

    def __findOrCreateCorrespondingType(self, typename):
        """ Based on the provided typename,
        find the closest matching type or base type.
        If no matching type was found
        a dynamically constructed metatype will be used

        @param typename: The type name
        @return: A type that matches the provided type name
        """
        subClasses = list(_allSubClasses(Object))
        baseTypes = self.baseTypes(typename)

        # Find exact python type
        for clazz in subClasses:
            if clazz.NAP_TYPE == typename:
                return clazz

        # Find closest base type and generate metatype
        for clazz in subClasses:
            napType = clazz.NAP_TYPE
            if napType in baseTypes:
                return self.__getOrCreateMetaType(typename, clazz)

        # No corresponding type found
        return self.__metatype(typename)

    def newObject(self, dic):
        """ Construct a new object based on a dict containing its data.
        If no corresponding implementation is found in this file,
        use a dynamically constructed meta type.

        @param dic: The Object data, according to the RPC format
        @return: An instance of thee
        """
        if _J_VALUE_TYPE in dic:
            Clazz = Attribute
        else:
            Clazz = self.__findOrCreateCorrespondingType(dic[_J_TYPE])

        return Clazz(self, dic)

    @staticmethod
    def toPythonValue(value, valueType):
        if valueType == 'bool':
            if value == 'true':
                return True
            if value == 'false':
                return False
            assert False
        if valueType == 'int':
            return int(value)
        if valueType == 'float':
            return float(value)
        return value

    @staticmethod
    def fromPythonValue(value):
        if isinstance(value, bool):
            return 'true' if value else 'false'
        return str(value)

    def handleMessageReceived(self, jsonMessage):
        """ Handle message coming back from RPC server
        Dynamically look up the method to call based on RPC method name.
        eg. A method 'nameChanged' will call method '_handle_nameChanged'

        @type jsonMessage: dict
        """
        # Construct method name
        handlerMethodName = '_handle_%s' % jsonMessage['id']
        result = jsonMessage['result']
        if result:
            if hasattr(self, handlerMethodName):
                # Call the method, pass unpacked dict entries
                getattr(self, handlerMethodName)(**json.loads(result))
            else:
                raise Exception(
                    'No handler for callback: %s' % handlerMethodName)
        self.messageReceived.emit()

    def __typenames(self, baseType=None):
        if not baseType:
            return self.__types
        return (t[0] for t in self.__types if baseType in t)

    ############################################################################
    ### Accessors
    ############################################################################

    def rpc(self):
        return self.__rpc

    def root(self):
        return self.__root

    def operatorTypes(self):
        return self.subTypes(Operator.NAP_TYPE, True)

    def dataTypes(self):
        return self.subTypes(Attribute.NAP_TYPE, True)

    def componentTypes(self):
        return self.subTypes(Component.NAP_TYPE, True)

    ############################################################################
    ### RPC Callback Handlers, signature must match server initiated calls
    ############################################################################

    def _handle_log(self, level, levelName, text):
        self.logMessageReceived.emit(level, levelName, text)

    def _handle_nameChanged(self, ptr, name):
        obj = self.findObject(ptr)
        obj.onNameChanged(name)

    def _handle_attributeValueChanged(self, ptr, name, value):
        attrib = self.findObject(ptr)
        value = self.toPythonValue(value, attrib.valueType())
        attrib._value = value
        attrib.valueChanged.emit(value)

    def _handle_objectAdded(self, ptr, child):
        parent = self.findObject(ptr)
        child = self.newObject(json.loads(child))
        parent.onChildAdded(child)

    def _handle_objectRemoved(self, ptr):
        child = self.findObject(ptr)
        self.objectRemoved.emit(child)
        parent = child.parent()
        parent.onChildRemoved(child)

    def _handle_getModuleInfo(self, **info):
        self.__types = info['types']
        self.moduleInfoChanged.emit(info)

    def _handle_getObjectTree(self, **jsonDict):
        # self.__root = self.toObjectTree(jsonDict)
        self.__root = self.newObject(jsonDict)
        self.rootChanged.emit()

    def _handle_plugConnected(self, srcPtr, dstPtr):
        srcPlug = self.findObject(srcPtr)
        assert isinstance(srcPlug, OutputPlugBase)
        dstPlug = self.findObject(dstPtr)
        assert isinstance(dstPlug, InputPlugBase)

        dstPlug.connected.emit(srcPlug, dstPlug)

    def _handle_plugDisconnected(self, srcPtr, dstPtr):
        srcPlug = self.findObject(srcPtr)
        assert isinstance(srcPlug, OutputPlugBase)
        dstPlug = self.findObject(dstPtr)
        assert isinstance(dstPlug, InputPlugBase)

        dstPlug.disconnected.emit(srcPlug, dstPlug)

    def _handle_copyObjectTree(self, **jsonDict):
        QApplication.clipboard().setText(json.dumps(jsonDict, indent=4))

    ############################################################################
    ### RPC Calls
    ############################################################################

    def triggerSignalAttribute(self, attrib):
        self.__rpc.triggerSignalAttribute(attrib.ptr())

    def removeObjects(self, objects):
        for o in objects:
            self.__rpc.removeObject(o.ptr())
        self.__waitForMessage()

    def addObjectCallbacks(self, obj):
        self.__rpc.addObjectCallbacks(self.rpc().identity, obj.ptr())

    def removeObjectCallbacks(self, obj):
        self.__rpc.removeObjectCallbacks(self.rpc().identity, obj.ptr())

    def loadModuleInfo(self):
        self.__rpc.getModuleInfo()
        self.loadObjectTree()
        self.__waitForMessage()

    def setAttributeValue(self, attrib, value):
        self.__rpc.setAttributeValue(attrib.ptr(), self.fromPythonValue(value))
        self.__waitForMessage()

    def loadObjectTree(self):
        self.__rpc.getObjectTree()
        self.__waitForMessage()

    def copyObjectTree(self, obj):
        self.__rpc.copyObjectTree(obj.ptr())
        self.__waitForMessage()

    def pasteObjectTree(self, parentObj, jsonString):
        self.__rpc.pasteObjectTree(parentObj.ptr(), jsonString)
        self.__waitForMessage()

    def connectPlugs(self, srcPlug, dstPlug):
        assert isinstance(srcPlug, OutputPlugBase)
        assert isinstance(dstPlug, InputPlugBase)
        self.__rpc.connectPlugs(srcPlug.ptr(), dstPlug.ptr())
        self.__waitForMessage()

    def disconnectPlug(self, dstPlug):
        assert isinstance(dstPlug, InputPlugBase)
        self.__rpc.disconnectPlug(dstPlug.ptr())

    def setName(self, obj, name):
        self.__rpc.setName(obj.ptr(), name)
        self.__waitForMessage()

    def exportObject(self, obj, filename):
        self.__rpc.exportObject(obj.ptr(), filename)

    def importObject(self, parentObj, filename):
        self.__rpc.importObject(parentObj.ptr(), filename)

    def loadFile(self, filename):
        self.__rpc.loadFile(filename)

    def modules(self):
        return self.__rpc.getModules()

    def addEntity(self, parentEntity):
        self.__rpc.addEntity(parentEntity.ptr())
        self.__waitForMessage()

    def addChild(self, entity, componentType):
        self.__rpc.addChild(entity.ptr(), str(componentType))
        self.__waitForMessage()


