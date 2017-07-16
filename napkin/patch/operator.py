import importlib
import importlib.util
import inspect
import json
import os
import types
from collections import OrderedDict
from typing import Iterable, Tuple

from patch import operators

TYPE_CONVERSION = {
    object: {
        str: lambda v: str(v)
    },
    str: {
        float: lambda v: float(v),
        int: lambda v: int(v),
    },
}
TYPE_CONVERSION_PASSTHROUGH = lambda v: v


def addTypeConverter(fromType, toType, func):
    assert not typeConverter(fromType, toType)
    TYPE_CONVERSION.setdefault(fromType, {}).setdefault(toType, func)


def typeConverter(fromType, toType):
    if fromType == toType:
        return TYPE_CONVERSION_PASSTHROUGH
    for fromT, conv in TYPE_CONVERSION.items():
        if not issubclass(fromType, fromT): continue
        for toT, cv in conv.items():
            if not issubclass(toType, toT): continue
            return cv


class Obj(object):
    def __init__(self, name):
        super(Obj, self).__init__()
        self.__children = []
        self.__name = None
        self.__parent = None
        self.setName(name)

    def name(self):
        return self.__name

    def setName(self, name):
        if not self.__parent:
            self.__name = name
        else:
            self.__name = self.__parent.__uniqueChildName(name)

    def path(self):
        p = [self.name()]
        if self.__parent:
            return self.__parent.path() + p
        return p

    def pathStr(self):
        return '/'.join(self.path())

    def add(self, obj):
        obj.__parent = self
        obj.setName(self.__uniqueChildName(obj.name()))
        self.__children.append(obj)
        return obj

    def _triggerInlet(self, name, func):
        return self.add(TriggerInlet(name, func))

    def _triggerOutlet(self, name):
        return self.add(TriggerOutlet(name))

    def _dataInlet(self, name, valueType, value):
        return self.add(DataInlet(name, valueType, value))

    def _dataOutlet(self, name, valueType, getter):
        return self.add(DataOutlet(name, valueType, getter))

    def children(self, typ=None):
        if typ is None: typ = Obj
        return (c for c in self.__children if isinstance(c, typ))

    def __uniqueChildName(self, name):
        newname = name
        i = 1
        while self.findChild(newname):
            newname = '%s%s' % (name, i)
            i += 1
        return newname

    def findChild(self, name):
        return next((c for c in self.__children if c.name() == name), None)

    def __typeName(self):
        return '%s.%s' % (self.__module__, self.__class__.__name__)

    def dict(self):
        dic = OrderedDict(
            name=self.name(),
            type=self.__typeName(),
        )
        if self.__children:
            dic['children'] = [c.dict() for c in self.__children]
        return dic


class Attr(Obj):
    def __init__(self, name, valueType, value):
        super(Attr, self).__init__(name)
        self.valueType = valueType
        self.__value = value

    def dict(self):
        return {**super(Attr, self).dict(), **OrderedDict(
            valueType=self.valueType.__name__,
            value=self.__value
        )}

    def __call__(self):
        return self.__value


class Meta(Obj):
    def __init__(self):
        super(Meta, self).__init__('meta')
        self.x = self.add(Attr('x', float, 0))
        self.y = self.add(Attr('y', float, 0))


class Outlet(Obj):
    def __init__(self, name):
        super(Outlet, self).__init__(name)


class Inlet(Obj):
    def __init__(self, name):
        super(Inlet, self).__init__(name)

    def __call__(self, *args):
        return self.connectedOutlet.eval() if self.connectedOutlet else self()


class DataOutlet(Outlet):
    def __init__(self, name, valueType, getter):
        super(DataOutlet, self).__init__(name)
        self.valueType = valueType
        self.__getter = getter

    def __call__(self):
        return self.__getter()


class DataInlet(Inlet):
    def __init__(self, name, valueType, value):
        super(DataInlet, self).__init__(name)
        assert not value is None
        self.valueType = valueType
        self.value = value
        self.connectedOutlet = None

    def __call__(self):
        if self.connectedOutlet:
            return self.connectedOutlet()
        return self.value

    def connect(self, outlet: DataOutlet):
        self.connectedOutlet = outlet

    def disconnect(self):
        self.connectedOutlet = None

    def dict(self):
        dic = {**super(DataInlet, self).dict(), **OrderedDict(
            value=self.value,
        )}
        if self.connectedOutlet:
            dic['connection'] = self.connectedOutlet.pathStr()
        return dic


class TriggerInlet(Inlet):
    def __init__(self, name, function):
        super(TriggerInlet, self).__init__(name)
        self.function = function


class TriggerOutlet(Outlet):
    def __init__(self, name):
        super(TriggerOutlet, self).__init__(name)
        # type: TriggerInlet
        self.connectedInlet = None

    def trigger(self):
        if self.connectedInlet:
            self.connectedInlet.function()

    def connect(self, inlet: TriggerInlet):
        self.connectedInlet = inlet

    def disconnect(self):
        self.connectedInlet = None

    def dict(self):
        return {**super(TriggerOutlet, self).dict(), **OrderedDict(
            connection=self.connectedInlet.pathStr() if self.connectedInlet else "",
        )}


class Operator(Obj):
    meta = Meta()

    def __init__(self):
        super(Operator, self).__init__(self.displayName)

    def inlets(self) -> Iterable[Inlet]:
        for inlet in self.children(Inlet):
            yield inlet

    def outlets(self) -> Iterable[Outlet]:
        for outlet in self.children(Outlet):
            yield outlet


class Patch(Obj):
    def __init__(self):
        super(Patch, self).__init__('patch')

    def addOperator(self, op: Operator):
        self.add(op)


def operatorsInModule(mod: types.ModuleType) -> Iterable[type]:
    moduleName = mod.__name__
    for name, member in inspect.getmembers(mod):
        if not inspect.isclass(member): continue
        if member == Operator: continue
        if not issubclass(member, Operator): continue
        yield member


def modulesInFolder(folder:str) -> Iterable[types.ModuleType]:
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

    def __init__(self):
        super(type(self), self).__init__()
        for arg, default in zip(spec.args, spec.defaults):
            self._dataInlet(arg, spec.annotations[arg], default)

        retype = spec.annotations['return']
        self._dataOutlet('out', retype, func)

    cls = type(func.__name__, (Operator,), {'__init__': __init__})
    cls.displayName = func.__name__
    return cls
