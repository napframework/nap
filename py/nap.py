import hashlib
import inspect
import json

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from asyncjsonclient import AsyncJSONClient
from utils import qtutils

_J_CHILDREN = 'children'
_J_ATTRIBUTES = 'attributes'
_J_NAME = 'name'
_J_TYPE = 'type'
_J_VALUE_TYPE = 'vType'
_J_VALUE = 'value'
_J_ATTRIBUTE_TYPE = 'nap::AttributeBase'
_J_ENTITY_TYPE = 'nap::Entity'
_J_PTR = 'ptr'
_J_FLAGS = 'flags'
_J_EDITABLE = 'editable'
_J_CONNECTION = 'connection'
_J_SUBTYPES = 'subtypes'
_J_BASETYPES = 'basetypes'
_J_INSTANTIABLE = 'instantiable'

def _allSubClasses(cls):
    all_subclasses = []

    for subclass in cls.__subclasses__():
        all_subclasses.append(subclass)
        all_subclasses.extend(_allSubClasses(subclass))

    return reversed(all_subclasses)


def _stripCPPNamespace(name):
    return name[name.rfind(':') + 1:]


class Core(QObject):
    """ Core represents a NAP Application structure.
    """

    rootChanged = pyqtSignal()
    moduleInfoChanged = pyqtSignal(dict)
    typeHierarchyChanged = pyqtSignal()
    objectRemoved = pyqtSignal(object)
    logMessageReceived = pyqtSignal(int, str, str)
    messageReceived = pyqtSignal()

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
            return True if 'true' else False
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

    def _handle_copyObjectTree(self, **jsonDict):
        QApplication.clipboard().setText(json.dumps(jsonDict, indent=4))

    ############################################################################
    ### RPC Calls
    ############################################################################


    def removeObjects(self, objects):
        for o in objects:
            self.__rpc.removeObject(o.ptr())



    def addObjectCallbacks(self, obj):
        self.__rpc.addObjectCallbacks(self.rpc().identity, obj.ptr())

    def removeObjectCallbacks(self, obj):
        self.__rpc.removeObjectCallbacks(self.rpc().identity, obj.ptr())

    def loadModuleInfo(self):
        self.__rpc.getModuleInfo()
        self.loadObjectTree()

    def setAttributeValue(self, attrib, value):
        self.__rpc.setAttributeValue(attrib.ptr(), self.fromPythonValue(value))

    def loadObjectTree(self):
        self.__rpc.getObjectTree()

    def copyObjectTree(self, obj):
        self.__rpc.copyObjectTree(obj.ptr())

    def pasteObjectTree(self, parentObj, jsonString):
        self.__rpc.pasteObjectTree(parentObj.ptr(), jsonString)

    def connectPlugs(self, srcPlug, dstPlug):
        assert isinstance(srcPlug, OutputPlugBase)
        assert isinstance(dstPlug, InputPlugBase)
        self.__rpc.connectPlugs(srcPlug.ptr(), dstPlug.ptr())

    def setName(self, obj, name):
        self.__rpc.setName(obj.ptr(), name)

    def exportObject(self, obj, filename):
        self.__rpc.exportObject(obj.ptr(), filename)

    def importObject(self, parentObj, filename):
        self.__rpc.importObject(parentObj.ptr(), filename)

    def modules(self):
        return self.__rpc.getModules()

    def addEntity(self, parentEntity):
        self.__rpc.addEntity(parentEntity.ptr())

    def addChild(self, entity, componentType):
        self.__rpc.addChild(entity.ptr(), str(componentType))


################################################################################
### NAP Type Wrappers
################################################################################

class Object(QObject):
    NAP_TYPE = 'nap::Object'

    class Flags(object):
        Visible = 1 << 0
        Editable = 1 << 1
        Removable = 1 << 2

    nameChanged = pyqtSignal(str)
    childAdded = pyqtSignal(object)
    childRemoved = pyqtSignal(object)

    def __init__(self, core, dic):
        super(Object, self).__init__()
        self._dic = dic
        self.__parent = None
        self.__core = core
        self.__flags = dic[_J_FLAGS]
        self.__ptr = dic[_J_PTR]
        self.__core.setObject(self.__ptr, self)
        self.__name = dic[_J_NAME]
        if _J_TYPE in dic:
            self.__typename = dic[_J_TYPE]
        else:
            self.__typename = dic[_J_VALUE_TYPE]

        self.__children = []
        if _J_CHILDREN in dic:
            for childDic in dic[_J_CHILDREN]:
                child = core.newObject(childDic)
                child.__parent = self
                self.__children.append(child)
        if _J_ATTRIBUTES in dic:
            for childDic in dic[_J_ATTRIBUTES]:
                child = core.newObject(childDic)
                child.__parent = self
                self.__children.append(child)

    def checkFlag(self, flag):
        return self.__flags & flag

    def isEditable(self):
        if self.parent():
            if not self.parent().isEditable():
                return False
            return self.checkFlag(Object.Flags.Editable)
        return self.checkFlag(Object.Flags.Editable)

    def path(self):
        if not self.parent():
            return []

        return self.parent().path() + [self.name()]

    def pathString(self):
        return '/%s' % '/'.join(self.path())

    def resolvePath(self, path):
        if isinstance(path, str):
            return self.resolvePath(path.split('/'))

        if not path[0]:
            path.pop(0)  # Strip root

        childName = path.pop(0)
        child = self.child(childName)
        if not child:
            return None

        if path:
            return child.resolvePath(path)
        else:
            return child

    def ptr(self):
        return self.__ptr

    def core(self):
        """
        @rtype: Core
        """
        return self.__core

    def child(self, name):
        for c in self.__children:
            if c.name() == name:
                return c

    def childOftype(self, typename):
        for c in self.__children:
            if c.typename() == typename:
                return c

    def onNameChanged(self, name):
        self.__name = name
        self.nameChanged.emit(name)

    def onChildAdded(self, child):
        child.__parent = self
        self.__children.append(child)
        self.childAdded.emit(child)

    def onChildRemoved(self, child):
        self.childRemoved.emit(child)
        self.__children.remove(child)

    def setName(self, name):
        self.core().setName(self, name)

    def name(self):
        if self.__name:
            return self.__name

    def typename(self):
        if self.__typename:
            return self.__typename

    def parent(self):
        return self.__parent

    def addChild(self, typename, name):
        return self.__rpc.addEntity(self.__ptr, typename, name);

    def addEntity(self, name):
        return Object(self.__core, self.__rpc.addEntity(self.__ptr, name))

    def children(self, objType=None):
        if not objType:
            return self.__children
        res = []
        for child in self.__children:
            if isinstance(child, objType):
                res.append(child)
        return res

    def __repr__(self):
        return '<%s> %s' % (self.typename(), self.name())


class Link(Object):
    NAP_TYPE = 'nap::Link'

    def __init__(self, *args):
        super(Link, self).__init__(*args)


class AttributeObject(Object):
    NAP_TYPE = 'nap::AttributeObject'

    def __init__(self, *args):
        super(AttributeObject, self).__init__(*args)

    def attributes(self):
        return self.children(Attribute)

    def attr(self, name):
        for at in self.attributes():
            if at.name() == name:
                return at
        raise AttributeError('No attribute with name %s' % name)


class Entity(AttributeObject):
    NAP_TYPE = 'nap::Entity'

    def __init__(self, *args):
        super(Entity, self).__init__(*args)


class Attribute(Object):
    NAP_TYPE = 'nap::AttributeBase'

    valueChanged = pyqtSignal(object)

    def __init__(self, *args):
        super(Attribute, self).__init__(*args)
        self._valueType = self._dic[_J_VALUE_TYPE]
        self._value = None
        if _J_VALUE in self._dic:
            self._value = self.core().toPythonValue(self._dic[_J_VALUE],
                                                    self.valueType())

    def setValue(self, value):
        self.core().setAttributeValue(self, value)

    def value(self):
        return self._value

    def valueType(self):
        return self._valueType


class Component(AttributeObject):
    NAP_TYPE = 'nap::Component'

    def __init__(self, *args):
        super(Component, self).__init__(*args)


class Operator(AttributeObject):
    NAP_TYPE = 'nap::Operator'

    def __init__(self, *args):
        super(Operator, self).__init__(*args)

    def inputPlugs(self):
        """
        @rtype: collections.Iterable[nap.InputPlugBase]
        """
        for child in self.children(InputPlugBase):
            yield child

    def outputPlugs(self):
        """
        @rtype: collections.Iterable[nap.OutputPlugBase]
        """
        for child in self.children(OutputPlugBase):
            yield child

    def connections(self):
        for outPlug in self.outputPlugs():
            raise Exception()


class Patch(AttributeObject):
    NAP_TYPE = 'nap::Patch'

    def __init__(self, *args):
        super(Patch, self).__init__(*args)


class Plug(Object):
    NAP_TYPE = 'nap::Plug'

    def __init__(self, *args):
        super(Plug, self).__init__(*args)
        self.__dataType = self._dic['dataType']

    def dataType(self):
        return self.__dataType


class InputPlugBase(Plug):
    NAP_TYPE = 'nap::InputPlugBase'

    connected = pyqtSignal(object, object)

    def __init__(self, *args):
        super(InputPlugBase, self).__init__(*args)
        self.__connection = ''
        if _J_CONNECTION in self._dic.keys():
            self.__connection = self._dic[_J_CONNECTION]

    def connection(self):
        if not self.__connection:
            return None
        return self.core().resolvePath(self.__connection)

    def connectTo(self, outPlug):
        self.core().connectPlugs(outPlug, self)


class OutputPlugBase(Plug):
    NAP_TYPE = 'nap::OutputPlugBase'

    disconnected = pyqtSignal(object)

    def __init__(self, *args):
        super(OutputPlugBase, self).__init__(*args)
