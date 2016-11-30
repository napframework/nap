import inspect
import json

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from asyncjsonclient import AsyncJSONClient

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


def _allSubClasses(cls):
    all_subclasses = []

    for subclass in cls.__subclasses__():
        all_subclasses.append(subclass)
        all_subclasses.extend(_allSubClasses(subclass))

    return reversed(all_subclasses)


def _stripCPPNamespace(name):
    return name[name.rfind(':') + 1:]


def toPythonValue(value, valueType):
    if valueType == 'bool':
        return True if 'true' else False
    if valueType == 'int':
        return int(value)
    if valueType == 'float':
        return float(value)
    return value


def fromPythonValue(value):
    if isinstance(value, bool):
        return 'true' if value else 'false'
    return str(value)


class Core(QObject):
    """ Core represents a NAP Application structure.
    """

    rootChanged = pyqtSignal()
    moduleInfoChanged = pyqtSignal(dict)
    typeHierarchyChanged = pyqtSignal()
    objectRemoved = pyqtSignal(object)
    logMessageReceived = pyqtSignal(int, str, str)
    messageReceived = pyqtSignal()

    def __init__(self, host='tcp://localhost:8888'):
        super(Core, self).__init__()
        self.__rpc = AsyncJSONClient(host)
        self.__rpc.messageReceived.connect(self.onMessageReceived)
        self.__componentTypes = None
        self.__dataTypes = None
        self.__operatorTypes = None
        self.__root = None
        self.__objects = {}
        self.__types = []
        self.__metatypes = {}

    def resolvePath(self, path):
        return self.root().resolvePath(path)

    def setObject(self, ptr, obj):
        self.__objects[ptr] = obj

    def findObject(self, ptr):
        """
        @type ptr: int
        @rtype: Object
        """
        if not isinstance(ptr, int):
            ptr = int(ptr)
        return self.__objects[ptr]

    def onLog(self, jsonDict):
        self.logMessageReceived.emit(jsonDict['level'], jsonDict['levelName'], jsonDict['text'])

    def onNameChanged(self, jsonDict):
        obj = self.findObject(jsonDict[_J_PTR])
        obj.onNameChanged(jsonDict[_J_NAME])

    def onMessageReceived(self, jsonMessage):
        """ Handle message coming back from RPC server
        @type jsonMessage: dict
        """
        id = jsonMessage['id']
        handler = 'on%s%s' % (id[0].upper(), id[1:])
        result = jsonMessage['result']
        if result:
            if hasattr(self, handler):
                getattr(self, handler)(json.loads(result))
            else:
                raise Exception('No handler for callback: %s' % handler)
        self.messageReceived.emit()

    def onAttributeValueChanged(self, jsonDict):
        attrib = self.findObject(jsonDict[_J_PTR])
        attrib._value = jsonDict[_J_VALUE]
        attrib.valueChanged.emit(attrib)

    def onObjectAdded(self, jsonDict):
        parent = self.findObject(jsonDict[_J_PTR])
        child = self.newObject(json.loads(jsonDict['child']))
        parent.onChildAdded(child)

    def onObjectRemoved(self, jsonDict):
        child = self.findObject(jsonDict[_J_PTR])
        self.objectRemoved.emit(child)
        parent = child.parent()
        parent.onChildRemoved(child)

    def onGetModuleInfo(self, info):
        self.__componentTypes = info['componentTypes']
        self.__operatorTypes = info['operatorTypes']
        self.__dataTypes = info['dataTypes']
        self.__types = info['types']
        self.moduleInfoChanged.emit(info)

    def onPlugConnected(self, jsonDict):
        srcPlug = self.findObject(jsonDict['srcPtr'])
        assert isinstance(srcPlug, OutputPlugBase)
        dstPlug = self.findObject(jsonDict['dstPtr'])
        assert isinstance(dstPlug, InputPlugBase)

        dstPlug.connected.emit(srcPlug)



    def types(self):
        return self.__types

    def baseTypes(self, typename):
        for t in self.__types:
            if t['name'] == typename:
                return t['baseTypes']

    def isSubClass(self, typename, baseTypename):
        for t in self.__types:
            if t == typename:
                if baseTypename in self.__types[t]:
                    return True
                return False
        return False

    def onGetObjectTree(self, jsonDict):
        # self.__root = self.toObjectTree(jsonDict)
        self.__root = self.newObject(jsonDict)
        self.rootChanged.emit()

    def onCopyObjectTree(self, jsonDict):
        QApplication.clipboard().setText(json.dumps(jsonDict, indent=4))

    def rpc(self):
        return self.__rpc

    def root(self):
        return self.__root

    def exportObject(self, obj, filename):
        self.__rpc.exportObject(obj.ptr(), filename)

    def importObject(self, parentObj, filename):
        self.__rpc.importObject(parentObj.ptr(), filename)

    def modules(self):
        return self.__rpc.getModules()

    def componentTypes(self, modulename=None):
        return self.__componentTypes

    def addEntity(self, parentEntity):
        self.__rpc.addEntity(parentEntity.ptr())

    def addChild(self, entity, componentType):
        self.__rpc.addChild(entity.ptr(), str(componentType))

    def removeObjects(self, objects):
        for o in objects:
            self.__rpc.removeObject(o.ptr())

    def operatorTypes(self, modulename=None):
        if modulename:
            return self.__rpc.getOperatorTypes(modulename)
        if self.__operatorTypes is None:
            self.__operatorTypes = self.__rpc.getOperatorTypes('')
        return self.__operatorTypes

    def dataTypes(self, modulename=None):
        if modulename:
            return self.__rpc.getDataTypes(modulename)
        if self.__dataTypes is None:
            self.__dataTypes = self.__rpc.getDataTypes('')
        return self.__dataTypespath

    def addObjectCallbacks(self, obj):
        self.__rpc.addObjectCallbacks(self.rpc().identity, obj.ptr())

    def removeObjectCallbacks(self, obj):
        self.__rpc.removeObjectCallbacks(self.rpc().identity, obj.ptr())

    def loadModuleInfo(self):
        self.__rpc.getModuleInfo()
        self.loadObjectTree()

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


    def __metaType(self, cppTypename, clazz=None):
        if not clazz:
            clazz = Object
        pythonTypename = _stripCPPNamespace(cppTypename)
        if not pythonTypename in self.__metatypes:
            t = type(pythonTypename, (clazz,), dict())
            print('Created metatype: %s' % t)
            return t

    def findCorrespondingType(self, typename):
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
                return self.__metaType(typename, clazz)

        # No corresponding type found
        return self.__metatype(typename)

    def newObject(self, dic):
        if _J_VALUE_TYPE in dic:
            clazz = Attribute
        else:
            clazz = self.findCorrespondingType(dic[_J_TYPE])

        return clazz(self, dic)


###############################################################################################
# The classes below wrap nap:: classes
###############################################################################################

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
        self.__rpc = core.rpc()
        self.core().addObjectCallbacks(self)

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

    def __del__(self):
        self.core().removeObjectCallbacks(self)

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
        self.__rpc.setName(self.__ptr, name)

    def name(self):
        if self.__name:
            return self.__name

    def typename(self):
        if self.__typename:
            return self.__typename
        if self.__ptr:
            return self.__rpc.getTypeName(self.__ptr)

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
        print('No attribute with name %s' % name)

    def forceSetAttributeValue(self, attrib, value):
        self.core().rpc().forceSetAttributeValue(self.core().rpc().identity, self.ptr(), attrib, fromPythonValue(value),
                                                 'int')


class Entity(AttributeObject):
    NAP_TYPE = 'nap::Entity'

    def __init__(self, *args):
        super(Entity, self).__init__(*args)


class Attribute(Object):
    NAP_TYPE = 'nap::Attribute'

    valueChanged = pyqtSignal(object)

    def __init__(self, *args):
        super(Attribute, self).__init__(*args)
        self._value = self._dic[_J_VALUE]
        self._valueType = None

    def setValue(self, value):
        self.core().rpc().setAttributeValue(self.ptr(), str(fromPythonValue(value)))

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
            pass


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

    connected = pyqtSignal(object)

    def __init__(self, *args):
        super(InputPlugBase, self).__init__(*args)



class OutputPlugBase(Plug):
    NAP_TYPE = 'nap::OutputPlugBase'

    disconnected = pyqtSignal(object)

    def __init__(self, *args):
        super(OutputPlugBase, self).__init__(*args)

    def connectTo(self, dstPlug):
        self.core().connectPlugs(self, dstPlug)