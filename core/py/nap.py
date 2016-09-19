import ctypes

import os

LIB_DIR = '.'
LIB_FILE = 'libnapcore.so'


def loadLibrary():
    filename = '%s/%s' % (LIB_DIR, LIB_FILE)
    os.environ['PATH'] = '%s;%s' % (LIB_DIR, os.environ['PATH'])
    lib = ctypes.CDLL(filename)
    lib.core_getDataTypeName.restype = ctypes.c_void_p
    lib.core_getComponentTypeName.restype = ctypes.c_void_p
    lib.core_getOperatorTypeName.restype = ctypes.c_void_p
    lib.object_getName.restype = ctypes.c_void_p
    lib.entity_addEntity.argTypes = [ctypes.c_char_p]

    return lib


def toPythonTypeName(typename):
    return str(typename).replace('::', '_')


def toPyString(ptr):
    """ Convert raw pointer to string and free it """
    v = ctypes.cast(ptr, ctypes.c_char_p).value
    NAPWrap.naplib.freeStr(ptr)
    return v


class NAPWrap(object):
    naplib = loadLibrary()

    def __init__(self, ptr):
        self._ptr = ptr


class TypeInfo(object):
    def __init__(self, core, rawName):
        self.__core = core
        self.__rawName = rawName

    def core(self):
        return self.__core

    def rawName(self):
        return self.__rawName

    def __str__(self):
        return '%s (%s)' % (self.__repr__(), self.rawName())


class Object(NAPWrap):
    def __init__(self, ptr):
        super(Object, self).__init__(ptr)

    def parent(self):
        return Object(self.naplib.object_getParent(self._ptr))

    def name(self):
        return toPyString(self.naplib.object_getName(self._ptr))

    def setName(self, name):
        self.naplib.object_setName(self._ptr, name)

    def addChild(self, typeName, name):
        return Object(self.naplib.object_addChild(typeName, name))

    def __eq__(self, other):
        return other._ptr == self._ptr


class AttributeObject(Object):
    def __init__(self, ptr):
        super(AttributeObject, self).__init__(ptr)


class Entity(AttributeObject):
    def __init__(self, ptr):
        super(Entity, self).__init__(ptr)

    def addEntity(self, name):
        return Entity(self.naplib.entity_addEntity(self._ptr, name))


class Core(NAPWrap):
    def __init__(self):
        super(Core, self).__init__(self.naplib.core_create())

    def root(self):
        return Entity(self.naplib.core_getRoot(self._ptr))

    def addEntity(self, name):
        return Entity(self.naplib.core_addEntity(self._ptr, name))

    def dataTypes(self):
        return self.__types(self.naplib.core_getDataTypeCount, self.naplib.core_getDataTypeName)

    def componentTypes(self):
        return self.__types(self.naplib.core_getComponentTypeCount, self.naplib.core_getComponentTypeName)

    def operatorTypes(self):
        return self.__types(self.naplib.core_getOperatorTypeCount, self.naplib.core_getOperatorTypeName)

    def __types(self, countFn, nameFn):
        for i in range(countFn(self._ptr)):
            yield TypeInfo(self, nameFn(self._ptr, i))

    def __del__(self):
        self.naplib.core_destroy(self._ptr)

    def initialize(self):
        self.naplib.core_initialize()


if __name__ == '__main__':
    core = Core()
    core.initialize()
    e = core.addEntity("MyEntity")
    print(e.name())
    child = e.addEntity('MyChild')
    child.setName('LeChild')
    print(child.parent())
