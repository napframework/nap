import importlib.util
import inspect
import logging
import os
from functools import lru_cache
from typing import Iterable, Optional
import nap


class Module:
    def __init__(self, mod):
        self.__module = mod

    def filename(self):
        return self.__module.__file__

    def name(self):
        try:
            return self.__module.NAP_MODULE_NAME
        except AttributeError as e:
            logging.error('%s (%s)' % (e, self))

    def __classes(self, typ=None):
        for name, obj in inspect.getmembers(self.__module):
            if not inspect.isclass(obj):
                continue
            if not typ:
                yield obj
            elif issubclass(obj, typ):
                yield obj

    def componentTypes(self):
        return (t for t in self.__classes(nap.Component))

    def operatorTypes(self):
        return (t for t in self.__classes(nap.Operator))

    def dataTypes(self):
        return self.__module.DATA_TYPES

    def __repr__(self):
        return '%s (%s)' % (type(self), self.__module)


class ModuleManager:
    def __init__(self, directory):
        self.__dir = os.path.abspath(directory)

    @staticmethod
    def __loadModule(filename: str) -> Optional[Module]:
        if not filename.endswith('.py'):
            return None

        logging.info('Loading module %s' % filename)
        basename = os.path.basename(filename)
        spec = importlib.util.spec_from_file_location(basename, filename)
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        return Module(mod)

    @lru_cache(maxsize=None)
    def modules(self) -> Iterable[Module]:
        logging.info('Loading modules from %s' % os.path.abspath(self.__dir))
        mods = []
        for f in os.listdir(self.__dir):
            filename = os.path.join(self.__dir, f)
            mod = self.__loadModule(filename)
            if mod:
                mods.append(mod)
        return mods

class Core(nap.Core):
    MODULE_DIR = './modules'

    def __init__(self):
        super(Core, self).__init__()
        self.__root = Entity()

    def root(self):
        return self.__root

    def refresh(self):
        self.moduleInfo.cache_clear()
        self.moduleInfoChanged.emit(self.moduleInfo())

    @lru_cache(maxsize=None)
    def moduleInfo(self) -> ModuleManager:
        path = os.path.abspath(self.MODULE_DIR)
        if not os.path.exists(path):
            raise FileNotFoundError(path)
        return ModuleManager(path)

    def componentTypes(self):
        return (typ for mod in self.moduleInfo().modules() for typ in mod.componentTypes())

    def operatorTypes(self):
        return (typ for mod in self.moduleInfo().modules() for typ in mod.operatorTypes())

    def dataTypes(self):
        return (typ for mod in self.moduleInfo().modules() for typ in mod.dataTypes())


class Entity(nap.Entity):



    def __init__(self):
        super(Entity, self).__init__()
