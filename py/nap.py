from PyQt5.QtCore import *

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


class TRIGGER(object):
    def __init__(self):
        pass

def _allSubClasses(cls):
    all_subclasses = []

    for subclass in cls.__subclasses__():
        all_subclasses.append(subclass)
        all_subclasses.extend(_allSubClasses(subclass))

    return reversed(all_subclasses)


def _stripCPPNamespace(name):
    return name[name.rfind(':') + 1:]


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
        return bool(self.__flags & flag)

    def isEditable(self):
        if self.parent() and not self.parent().isEditable():
            return False
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
        @rtype: core.Core
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
        if child in self.__children:
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
        if self._valueType == 'nap::SignalAttribute':
            self._value = TRIGGER

    def setValue(self, value):
        if self._value == TRIGGER:
            self.core().triggerSignalAttribute(self)
        else:
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
    disconnected = pyqtSignal(object, object)

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

    def disconnect(self):
        self.core().disconnectPlug(self)


class OutputPlugBase(Plug):
    NAP_TYPE = 'nap::OutputPlugBase'

    def __init__(self, *args):
        super(OutputPlugBase, self).__init__(*args)
