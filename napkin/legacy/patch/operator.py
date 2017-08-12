import importlib
import importlib.util
import inspect
import json
import os
import types
from collections import OrderedDict
from typing import Iterable, Tuple, Union

import collections

from patch import operators

TYPE_CONVERSION = {
    object: {
        str: lambda v: str(v),
        # bool: lambda v

    },
    str: {
        float: lambda v: float(v),
        int: lambda v: int(v),
        bool: lambda v: bool(v),
    },
    float: {
        int: lambda v: int(v),
        bool: lambda v: bool(v),
    },
    bool: {
        int: lambda v: int(v),
        float: lambda v: float(v),
    },
    int: {
        float: lambda v: float(v),
        bool: lambda b: bool(int),
    }
}
TYPE_CONVERSION_PASSTHROUGH = lambda v: v
PYTHON_TYPES = (float, int, str, bool)


def convertToPythonType(v, typ):
    try:
        typ(v)
    except Exception as e:
        print(e)
        return None


def addTypeConverter(fromType, toType, func):
    assert not typeConverter(fromType, toType)
    TYPE_CONVERSION.setdefault(fromType, {}).setdefault(toType, func)


def typeConverter(fromType, toType):
    # same types
    if fromType == toType:
        return TYPE_CONVERSION_PASSTHROUGH
    # any python type to object
    if fromType in PYTHON_TYPES and issubclass(toType, object):
        return TYPE_CONVERSION_PASSTHROUGH

    # any object to python type
    if issubclass(toType, object) and fromType in PYTHON_TYPES:
        return lambda v: convertToPythonType(v, toType)

    for fromT, conv in TYPE_CONVERSION.items():
        if not issubclass(fromType, fromT): continue
        for toT, cv in conv.items():
            if not issubclass(toType, toT): continue
            return cv


class OrderedClass(type):
    @classmethod
    def __prepare__(mcs, name, bases):
        return OrderedDict()

    def __new__(cls, name, bases, classdict):
        result = type.__new__(cls, name, bases, dict(classdict))
        result.__fields__ = list(classdict.keys())
        return result


class Obj(object, metaclass=OrderedClass):
    def __init__(self, parent):
        super(Obj, self).__init__()
        self.__parent = parent
        self.__meta = None

    def meta(self):
        if not self.__meta:
            self.__meta = Meta()
        return self.__meta

    def addAttr(self, name, valueType, value):
        self.add(Attr(name, valueType, valueType))

    def name(self) -> str:
        if self.__parent:
            return next((k for k, v in self.__parent.children() if v == self), None)
        return ''

    def add(self, name, obj):
        obj.__parent = self
        name = self.__uniqueChildName(name)
        assert not name in dir(self)
        if not name in self.__dict__:
            self.__dict__[name] = obj
        return obj

    def children(self, typ=None):
        if typ is None: typ = Obj
        for k, v in self.__dict__.items():
            if k.startswith('_'): continue
            if not isinstance(v, typ): continue
            yield k, v

    def __uniqueChildName(self, name):
        newname = name
        i = 1
        while newname in dir(self):
            newname = '%s%s' % (name, i)
            i += 1
        return newname

    def findChild(self, name):
        return self.__dict__.get(name, None)

    def __typeName(self):
        return '%s.%s' % (self.__module__, self.__class__.__name__)

    def path(self) -> str:
        if self.__parent:
            return '%s/%s' % (self.__parent.path(), self.name())
        return self.name()

    def dict(self):
        # TODO: Link resolve
        dic = OrderedDict()
        for k, v in self.children():
            dic[k] = v.dict()
        dic['__type'] = self.__typeName()
        return dic


class Link(object):
    def __init__(self):
        super(Link, self).__init__()
        self.__target = None

    def set(self, target: Obj):
        self.__target = target

    def target(self) -> Union[Obj, None]:
        return self.__target

    def clear(self):
        self.__target = None


class Attr(Obj):
    def __init__(self, parent: Obj, valueType, value):
        super(Attr, self).__init__(parent)
        self.valueType = valueType
        self.value = value

    def __call__(self, value=None):
        if value:
            self.value = value
        return self.value


class Meta(Obj):
    def __init__(self):
        super(Meta, self).__init__()


class Outlet(Obj):
    def __init__(self, parent: Obj):
        super(Outlet, self).__init__(parent)


class Inlet(Obj):
    def __init__(self, parent: Obj):
        super(Inlet, self).__init__(parent)

    def __call__(self, *args):
        return self.connectedOutlet.eval() if self.connectedOutlet else self()


class DataOutlet(Outlet):
    def __init__(self, parent: Obj, valueType, getter):
        super(DataOutlet, self).__init__(parent)
        self.valueType = valueType
        self.__getter = getter

    def __call__(self):
        return self.__getter()


class DataInlet(Inlet):
    def __init__(self, parent: Obj, valueType, value):
        super(DataInlet, self).__init__(parent)
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
    def __init__(self, parent: Obj, func):
        super(TriggerInlet, self).__init__(parent)
        self.function = func


class TriggerOutlet(Outlet):
    def __init__(self, parent: Obj):
        super(TriggerOutlet, self).__init__(parent)
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


class Operator(Obj):
    def __init__(self, parent: Obj):
        super(Operator, self).__init__(parent)
        self.x = Attr(self, float, 0)
        self.y = Attr(self, float, 0)

    def inlets(self) -> Iterable[Inlet]:
        for inlet in self.children(Inlet):
            yield inlet

    def outlets(self) -> Iterable[Outlet]:
        for outlet in self.children(Outlet):
            yield outlet


class Patch(Obj):
    def __init__(self, parent=None):
        super(Patch, self).__init__(parent)

    def addOperator(self, op: Operator):
        self.add(op)


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
        if not ext == '.py': continue
        if name == '__init__': continue
        yield importlib.import_module('patch.operators.%s' % name, f)
        # spec = importlib.util.spec_from_file_location('modname', folder)
        # foo = importlib.util.module_from_spec(spec)
        # return spec.loader.exec_module(foo)


def serializeAllOperatorsMeta() -> str:
    operators = [op.dict() for op in allOperators()]
    return json.dumps(operators, indent=2)


def allOperators() -> Iterable[Operator]:
    for mod in modulesInFolder(os.path.dirname(operators.__file__)):
        for op in operatorsInModule(mod):
            yield op


def operatorFromFunction(func):
    spec = inspect.getfullargspec(func)

    def __init__(self, parent: Obj):
        super(type(self), self).__init__(parent)
        for arg, default in zip(reversed(spec.args), reversed(spec.defaults)):
            self.__dict__[arg] = DataInlet(self, spec.annotations[arg], default)

        retype = spec.annotations['return']
        self.__dict__['out'] = DataOutlet(self, retype, func)

    cls = type(func.__name__, (Operator,), {'__init__': __init__})
    cls.displayName = func.__name__
    return cls
