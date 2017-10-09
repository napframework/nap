import importlib
import importlib.util
import inspect
import json
import os
import types
from collections import OrderedDict
from types import FunctionType
from typing import Iterable, Union, Tuple

from PyQt5.QtCore import QObject, pyqtSignal

from napkin.models import modules



class Object(QObject):
    childAdded = pyqtSignal(object)
    childRemoved = pyqtSignal(object)

    def __init__(self, name: str = None):
        super(Object, self).__init__()
        self._parent = None
        self._children = []
        self._meta = None
        if name:
            self.setName(name)
        else:
            self.setName(type(self).__name__)

    def meta(self):
        if not self.__meta:
            self.__meta = Meta()
        return self.__meta

    def addAttr(self, name, valueType, defaultValue):
        attr = Attr(name, valueType, defaultValue)
        self.addChild(attr)
        return attr

    def name(self) -> str:
        return self.objectName()

    def setName(self, name):
        assert (isinstance(name, str))
        if self.parent():
            name = self.__uniqueName(name)
        self.setObjectName(name)

    def setParent(self, p: QObject):
        super(Object, self).setParent(p)
        self.setName(self.name())

    def addChild(self, objOrType):
        if isinstance(objOrType, type):
            assert issubclass(objOrType, Object)
            objOrType = objOrType()

        objOrType.__parent = self
        self._children.append(objOrType)
        return objOrType

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

    def __typeName(self):
        return '%s.%s' % (self.__module__, self.__class__.__name__)

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

    def pathStr(self) -> str:
        return '/' + '/'.join(self.path())

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
        for k, v in self.children():
            dic[k] = v.dict()
        dic['__type'] = self.__typeName()
        return dic

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
        self.valueType = valueType
        self.__getter = getter

    def __call__(self):
        return self.__getter()


class DataInlet(Inlet):
    def __init__(self, name: str, valueType: type, value):
        super(DataInlet, self).__init__(name)
        # assert not value is None
        self.valueType = valueType
        self.value = value
        self.__connection = Link()

    def __call__(self):
        if self.__connection.target():
            return self.__connection.target()()
        return self.value

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
        if self.__connection:
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
                for arg, default in zip(reversed(spec.args),
                                        reversed(spec.defaults)):
                    self.addChild(DataInlet(arg, spec.annotations[arg], default))

                # Create data outlet as from return value
                retypes = spec.annotations['return']
                # support multiple return values
                if not isinstance(retypes, Tuple):
                    retypes = [retypes]
                for i, retype in enumerate(retypes):
                    if returnLabels and i < len(retypes):
                        label = returnLabels[i]
                    else:
                        label = 're%s' % i
                    self.addChild(DataOutlet(label, retype, func))

            cls = type(func.__name__, (Operator,), {'__init__': __init__})
            cls.displayName = func.__name__
            return cls

        return super(Operator, cls).__new__(cls)

    def __init__(self, name=None, returnLabels=None):
        super(Operator, self).__init__(name)
        self.x = self.addAttr('x', float, 0)
        self.y = Attr('y', float, 0)

    @classmethod
    def displayName(cls):
        return cls.__name__

    def addDataInlet(self, name:str, valueType:type, defaultValue) -> DataInlet:
        return self.addChild(DataInlet(name, valueType, defaultValue))

    def addDataOutlet(self, name:str, valueType:type, getter) -> DataOutlet:
        return self.addChild(DataOutlet(name, valueType, getter))

    def addTriggerInlet(self, name:str, fn) -> TriggerInlet:
        return self.addChild(TriggerInlet(name, fn))

    def addTriggerOutlet(self, name:str) -> TriggerOutlet:
        return self.addChild(TriggerOutlet(name))

    def inlets(self) -> Iterable[Inlet]:
        for inlet in (c for c in self._children if isinstance(c, Inlet)):
            yield inlet

    def outlets(self) -> Iterable[Outlet]:
        for outlet in (c for c in self._children if isinstance(c, Outlet)):
            yield outlet


class Patch(Object):
    def __init__(self, name: str = None):
        super(Patch, self).__init__(name)

    def addOperator(self, op: Union[Operator, type, str]):
        assert (isinstance(op, Operator) or issubclass(op, Operator)) or isinstance(op, str)
        if isinstance(op, str):
            op = operatorType(str)
            assert op
        return self.addChild(op)


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

def operatorType(typename:str) -> Union[type, None]:
    return next((t for t in operatorTypes() if t.__name__ == typename), None)

def operatorFromFunction(func):
    spec = inspect.getfullargspec(func)
    def __init__(self):
        super(type(self), self).__init__()
        for arg, default in zip(reversed(spec.args), reversed(spec.defaults)):
            self.addChild(DataInlet(arg, spec.annotations[arg], default))

        retype = spec.annotations['return']
        self.addChild(DataOutlet('return', retype, func))

    cls = type(func.__name__, (Operator,), {'__init__': __init__})
    cls.displayName = func.__name__
    return cls

