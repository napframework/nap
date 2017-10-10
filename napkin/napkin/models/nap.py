import importlib
import importlib.util
import inspect
import json
import logging
import os
import types
from collections import OrderedDict
from types import FunctionType
from typing import Iterable, Union, Tuple

from PyQt5.QtCore import QObject, pyqtSignal

from napkin.models import modules

LOGGER = logging.root

NAP_MODULE_ID = 'nap'

_KEY_TYPE = 'type'
_KEY_NAME = 'name'
_KEY_CHILDREN = 'children'
_KEY_VALUE_TYPE = 'valueType'
_KEY_DEFAULT_VALUE = 'defaultValue'
_KEY_FUNCTION = 'function'
_KEY_INPUTVALUES = 'inletValues'
_KEY_CONNECTIONS = 'connections'

class Object(QObject):
    childAdded = pyqtSignal(object)  # child
    childRemoved = pyqtSignal(object)  # child
    added = pyqtSignal(object)  # parent
    removed = pyqtSignal(object)  # parent

    def __init__(self, name: str = None):
        super(Object, self).__init__()
        self._parent = None
        self._children = []
        self._meta = None
        self.__name = None
        if name:
            self.setName(name)
        else:
            self.setName(type(self).__name__)

    @classmethod
    def typeName(cls):
        modulename = cls.__module__[cls.__module__.rfind('.') + 1:]
        classname = cls.__name__
        return '%s.%s' % (modulename, classname)

    def meta(self):
        if not self.__meta:
            self._meta = Meta()
        return self.__meta

    def addAttr(self, name, valueType, defaultValue):
        attr = Attr(name, valueType, defaultValue)
        self.addChild(attr)
        return attr

    def name(self) -> str:
        return self.__name

    def setName(self, name):
        assert (isinstance(name, str))
        if self.parent():
            name = self.__uniqueName(name)
        self.__name = name

    def addChild(self, objOrType):
        assert self != objOrType
        assert objOrType
        if isinstance(objOrType, type):
            assert issubclass(objOrType, Object)
            objOrType = objOrType()

        objOrType._parent = self
        objOrType.setName(self.__uniqueName(objOrType.name()))
        self._children.append(objOrType)
        objOrType.added.emit(self)
        return objOrType

    def children(self):
        return self._children

    def childrenByType(self, childType: type) -> Iterable:
        return (c for c in self._children if isinstance(c, childType))

    def getChild(self, path):
        if not path: return None
        if isinstance(path, str): path = path.split('/')

        name = path.pop(0)
        if not name:
            return self.root().getChild(path)
        elif name == '..':
            # parent
            parent = self.parent()
            if not path:
                return parent
            elif parent:
                return parent.getChild(path)
        else:
            # child
            child = next((c for c in self.children() if c.name() == name), None)
            if path and child:
                return child.getChild(path)
            return child

    def parent(self):
        return self._parent

    def __uniqueName(self, name):
        if not self.parent():
            return name

        newname = name
        i = 1
        childrenNames = [c.name() for c in self.parent().children() if
                         c != self]
        while newname in childrenNames:
            newname = '%s%s' % (name, i)
            i += 1
        return newname

    def isRoot(self):
        return not bool(self.parent())

    def root(self):
        if not self.parent():
            return self
        return self.parent().root()

    def path(self) -> list:
        if not self.parent():
            return []
        parentPath = [] if self.parent().isRoot() else self.parent().path()
        return parentPath + [self.name()]

    def pathStr(self, fromObject=None) -> str:
        if fromObject:
            path = fromObject.pathTo(self)
        else:
            path = self.path()
        return '/' + '/'.join(path)

    def pathTo(self, other) -> list:
        up = []
        down = []
        for a, b in zip(self.path(), other.path()):
            if a == b:
                down.append(a)
                up.append('..')
        return up + down

    def dict(self):
        # TODO: Link resolve
        dic = OrderedDict()
        dic[_KEY_NAME] = self.name()
        dic[_KEY_TYPE] = type(self).typeName()
        dic[_KEY_CHILDREN] = [c.dict() for c in self.children()]
        return dic

    @classmethod
    def fromDict(cls, dic: dict):
        t = typeFromString(dic[_KEY_TYPE])
        o = t()
        assert (isinstance(o, Object))
        o.setName(dic[_KEY_NAME])
        for c in dic[_KEY_CHILDREN]:
            childType = typeFromString(c[_KEY_TYPE])
            child = childType.fromDict(c)
            o.addChild(child)

        return o

    def __repr__(self):
        return '%s(%s)' % (super(Object, self).__repr__(), self.name())


class Link(object):
    def __init__(self):
        super(Link, self).__init__()
        self.__target = None

    def set(self, target: Object):
        self.__target = target

    def target(self) -> Union[Object, None]:
        return self.__target

    def clear(self):
        self.__target = None


class Attr(Object):
    def __init__(self, name: str, valueType, defaultValue):
        super(Attr, self).__init__(name)
        self.valueType = valueType
        self.defaultValue = defaultValue
        self.value = defaultValue

    def __call__(self, value=None):
        if value:
            self.value = value
        return self.value


class Meta(Object):
    def __init__(self):
        super(Meta, self).__init__()


class Outlet(Object):
    def __init__(self, name: str = None):
        super(Outlet, self).__init__(name)


class Inlet(Object):
    def __init__(self, name: str = None):
        super(Inlet, self).__init__(name)

    def __call__(self, *args):
        return self.connectedOutlet.eval() if self.connectedOutlet else self()


class DataOutlet(Outlet):
    def __init__(self, name: str, valueType: type, getter):
        super(DataOutlet, self).__init__(name)
        assert valueType
        self.valueType = valueType
        self.__getter = getter

    def __call__(self):
        return self.__getter()


class DataInlet(Inlet):
    def __init__(self, name: str, valueType: type, value):
        super(DataInlet, self).__init__(name)
        # assert not value is None
        self.valueType = valueType
        self.__value = value
        self.__defaultValue = value
        self.__connection = Link()

    def defaultValue(self):
        return self.__defaultValue

    def value(self):
        """Unevaludated value"""
        return self.__value

    def setValue(self, v):
        assert(isinstance(v, self.valueType))
        self.__value = v

    def __call__(self):
        if self.__connection.target():
            return self.__connection.target()()
        return self.value()

    def isConnected(self):
        return bool(self.__connection.target())

    def connection(self):
        return self.__connection.target()

    def connect(self, outlet: DataOutlet):
        self.__connection.set(outlet)

    def disconnect(self):
        self.__connection.clear()

        # def dict(self):
        #     dic = {**super(DataInlet, self).dict(), **OrderedDict(
        #         value=self.value,
        #     )}
        #     if self.connectedOutlet:
        #         dic['connection'] = self.connectedOutlet.pathStr()
        #     return dic


class TriggerInlet(Inlet):
    def __init__(self, name, func):
        super(TriggerInlet, self).__init__(name)
        self.function = func


class TriggerOutlet(Outlet):
    def __init__(self, name):
        super(TriggerOutlet, self).__init__(name)
        # type: TriggerInlet
        self.__connection = Link()

    def trigger(self):
        if self.__connection:
            self.__connection.target().function()

    def __call__(self):
        if self.__connection and self.__connection.target():
            self.__connection.target().function()

    def isConnected(self):
        return bool(self.__connection.target())

    def connection(self):
        return self.__connection.target()

    def connect(self, inlet: TriggerInlet):
        self.__connection.set(inlet)

    def disconnect(self):
        self.__connection.clear()

        # def dict(self):
        #     return {**super(TriggerOutlet, self).dict(), **OrderedDict(
        #         connection=self.connectedInlet.pathStr() if self.connectedInlet else "",
        #     )}


class Operator(Object):
    """Operator can be instantiated or used as a decorator on a function"""

    def __new__(cls, name=None, returnLabels=None):
        if isinstance(name, FunctionType):
            # Used as decorator
            func = name
            spec = inspect.getfullargspec(func)

            def __init__(self):
                super(type(self), self).__init__()

                # Create data inlets from arguments
                if len(spec.args) > 0:
                    if not spec.defaults:
                        raise Exception(
                            'Operator function "%s" has %s arguments, but no defaults' % (
                                func.__name__, len(spec.args)))

                    if len(spec.args) != len(spec.defaults):
                        raise Exception(
                            'Operator function "%s" has %s arguments, but %s defaults' % (
                                func.__name__, len(spec.args),
                                len(spec.defaults)))

                for arg, default in zip(spec.args, spec.defaults):
                    self.addChild(
                        DataInlet(arg, spec.annotations[arg], default))

                # Create data outlet as from return value
                retypes = spec.annotations['return']
                # support multiple return values
                if not isinstance(retypes, Tuple):
                    if retypes is None:
                        raise Exception('Return type may not be None (%s)',
                                        func.__name__)
                    retypes = [retypes]
                for i, retype in enumerate(retypes):
                    if returnLabels and i < len(retypes):
                        label = returnLabels[i]
                    else:
                        if len(retypes) == 1:
                            label = 'result'
                        else:
                            label = 'result %s' % i
                    self.addChild(DataOutlet(label, retype, func))

            cls = type(func.__name__, (Operator,), {'__init__': __init__})
            return cls

        return super(Operator, cls).__new__(cls)

    def __init__(self, name=None, returnLabels=None):
        super(Operator, self).__init__(name)
        self.x = self.addAttr('x', float, 0)
        self.y = Attr('y', float, 0)

    @classmethod
    def displayName(cls):
        return cls.__name__

    def addDataInlet(self, name: str, valueType: type,
                     defaultValue) -> DataInlet:
        return self.addChild(DataInlet(name, valueType, defaultValue))

    def addDataOutlet(self, name: str, valueType: type, getter) -> DataOutlet:
        return self.addChild(DataOutlet(name, valueType, getter))

    def addTriggerInlet(self, name: str, fn) -> TriggerInlet:
        return self.addChild(TriggerInlet(name, fn))

    def addTriggerOutlet(self, name: str) -> TriggerOutlet:
        return self.addChild(TriggerOutlet(name))

    def inlets(self) -> Iterable[Inlet]:
        for inlet in (c for c in self._children if isinstance(c, Inlet)):
            yield inlet

    def dataInlets(self) -> Iterable[DataInlet]:
        return (n for n in self.inlets() if isinstance(n, DataInlet))

    def outlets(self) -> Iterable[Outlet]:
        for outlet in (c for c in self._children if isinstance(c, Outlet)):
            yield outlet

    def triggerOutlets(self) -> Iterable[TriggerOutlet]:
        return (t for t in self.outlets() if isinstance(t, TriggerOutlet))

    def dict(self):
        dic = OrderedDict()
        dic[_KEY_NAME] = self.name()
        dic[_KEY_TYPE] = type(self).typeName()

        # values and connections
        values = OrderedDict()
        connections = OrderedDict()

        for d in self.dataInlets():
            if d.value() != d.defaultValue():
                values[d.name()] = d.value()
            if d.isConnected():
                connections[d.name()] = d.connection().pathStr()

        for t in self.triggerOutlets():
            if t.isConnected():
                connections[t.name()] = t.connection().pathStr()

        if values:
            dic[_KEY_INPUTVALUES] = values
        if connections:
            dic[_KEY_CONNECTIONS] = connections

        return dic

    @classmethod
    def fromDict(cls, dic: dict):
        t = typeFromString(dic[_KEY_TYPE])
        op = t()
        return op

class Patch(Object):
    started = pyqtSignal()

    def __init__(self, name: str = None):
        super(Patch, self).__init__(name)

    def run(self):
        self.started.emit()

    def addOperator(self, op: Union[Operator, type, str]) -> Operator:
        assert (isinstance(op, Operator)
                or issubclass(op, Operator)) or isinstance(op, str)
        if isinstance(op, str):
            op = operatorType(str)
            assert op
        return self.addChild(op)

    def operators(self) -> Iterable[Operator]:
        return self.childrenByType(Operator)


def operatorsInModule(mod: types.ModuleType) -> Iterable[type]:
    moduleName = mod.__name__
    for name, member in inspect.getmembers(mod):
        if not inspect.isclass(member): continue
        if member == Operator: continue
        if not issubclass(member, Operator): continue
        yield member


def modulesInFolder(folder: str) -> Iterable[types.ModuleType]:
    for f in os.listdir(folder):
        name, ext = os.path.splitext(f)
        if not ext == '.py' or name == '__init__':
            continue
        from napkin.models import modules
        # parentmodule = '%s.%s' % (
        # modules.__name__, modules.__class__.__name__)
        yield importlib.import_module('%s.%s' % (modules.__name__, name), f)
        # spec = importlib.util.spec_from_file_location('modname', folder)
        # foo = importlib.util.module_from_spec(spec)
        # return spec.loader.exec_module(foo)


def serializeAllOperatorsMeta() -> str:
    operators = [op.dict() for op in operatorTypes()]
    return json.dumps(operators, indent=2)


def operatorTypes() -> Iterable[Operator]:
    for mod in modulesInFolder(os.path.dirname(modules.__file__)):
        for op in operatorsInModule(mod):
            yield op


def allTypes() -> Iterable[Object]:
    types = set()
    types.add(Object)
    for t in operatorTypes():
        types.add(t)
    for t in Object.__subclasses__():
        types.add(t)
    return types


def typeFromString(typename: str) -> Union[type, None]:
    return next((t for t in allTypes() if t.typeName() == typename), None)


def operatorType(typename: str) -> Union[type, None]:
    return next((t for t in operatorTypes() if t.__qualname__ == typename),
                None)


def operatorFromFunction(func) -> type:
    spec = inspect.getfullargspec(func)

    def __init__(self):
        super(type(self), self).__init__()
        for arg, default in zip(spec.args, spec.defaults):
            self.addChild(DataInlet(arg, spec.annotations[arg], default))

        retype = spec.annotations['return']
        self.addChild(DataOutlet('return', retype, func))

    cls = type(func.__name__, (Operator,), {'__init__': __init__})
    cls.displayName = func.__name__
    return cls


def dumps(obj: Object):
    return json.dumps(obj.dict(), indent=4)


def loads(data: str):
    data = json.loads(data)
    return Object.fromDict(data)
