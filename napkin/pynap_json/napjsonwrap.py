import json
import os
from collections import OrderedDict


class NAPType(object):
    def __init__(self, name, data):
        super(NAPType, self).__init__()
        self.name = name
        self.__data = data

    def properties(self):
        propsdic = self.__data['properties']
        print('=============================')
        for name in propsdic:
            print(name)
            yield NAPProperty(self, name, propsdic[name])

    def __repr__(self):
        return '<NAPType (%s)>' % self.name


class NAPProperty(object):
    def __init__(self, type, name, data):
        super(NAPProperty, self).__init__()
        self.__type = type
        self.name = name
        self.__data = data


class NAPInstance(object):
    def __init__(self, data):
        super(NAPInstance, self).__init__()
        self.__data = data
        self.__type = schema().typeFromName(self.typeName())

    def name(self):
        return self.__data['mID']

    def typeName(self):
        return self.__data['Type']

    def type(self) -> NAPType:
        return self.__type

    def hasValue(self, key):
        return key in self.__data

    def value(self, key):
        return self.__data[key]

    def setValue(self, name, valuestr):
        self.__data[name] = valuestr

    @staticmethod
    def fromDict(dic):
        inst = NAPInstance(dic)
        return inst



__SCHEMA = None


def schema():
    global __SCHEMA
    if not __SCHEMA:
        __SCHEMA = NAPSchema()
    return __SCHEMA


class NAPSchema(object):
    def __init__(self, filename=None):
        super(NAPSchema, self).__init__()
        self.__filename = filename or '%s/../schema/napschema.json' % os.path.dirname(__file__)
        self.__schema = None
        self.__types = set()
        self.readSchema()

    def readSchema(self):
        with open(self.__filename, 'r') as fp:
            self.__schema = json.load(fp)
        for k, v in self.__schema['definitions'].items():
            self.__types.add(NAPType(k, v))

    def types(self):
        return self.__types

    def typeFromName(self, name):
        return next((t for t in self.__types if t.name == name), None)


def loadFile(filename):
    with open(filename, 'r') as fp:
        data = json.load(fp, object_pairs_hook=OrderedDict)
    objects = []
    for ob in data['Objects']:
        objects.append(NAPInstance.fromDict(ob))
    return objects