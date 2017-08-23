import json
import os
import re
from collections import OrderedDict
from typing import Iterable

from pynap_json.constants import UNKNOWN_TYPE, PROP_COMPONENTS, PROP_CHILDREN


class NAPProperty(object):
    def __init__(self, ownerType, name: str, data: OrderedDict):
        super(NAPProperty, self).__init__()
        self.__ownerType = ownerType
        self.name = name
        self.__data = data

    def type(self) -> str:
        return self.__data.get('type', UNKNOWN_TYPE)


class NAPType(object):
    def __init__(self, name: str, data: OrderedDict):
        super(NAPType, self).__init__()
        self.name = name
        self.__data = data

    def properties(self) -> Iterable[NAPProperty]:
        propsdic = self.__data['properties']
        for name in propsdic:
            yield NAPProperty(self, name, propsdic[name])

    def __repr__(self):
        return '<NAPType (%s)>' % self.name


class NAPObject(object):
    def __init__(self, data):
        super(NAPObject, self).__init__()
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

    def components(self):
        """
        :rtype: Iterable[NAPObject]
        """
        return (NAPComponent(c) for c in self.__data.get(PROP_COMPONENTS, ()))

    def children(self):
        """
        :rtype: Iterable[NAPObject]
        """
        return (NAPObject(c) for c in self.__data.get(PROP_CHILDREN, ()))


    @staticmethod
    def fromDict(dic):
        return NAPObject(dic)

    def toDict(self):
        return self.__data


class NAPComponent(NAPObject):
    def __init__(self, data):
        super(NAPComponent, self).__init__(data)
        self.__nameRegex = re.compile(r".+::(.+)+?")

    def name(self):
        m =  self.__nameRegex.match(self.typeName())

        return re.sub('Component$', '', m.group(1))


__SCHEMA = None


def schema():
    global __SCHEMA
    if not __SCHEMA:
        __SCHEMA = NAPSchema()
    return __SCHEMA


class NAPSchema(object):
    def __init__(self, filename=None):
        super(NAPSchema, self).__init__()
        self.__filename = filename or '%s/../schema/napschema.json' % os.path.dirname(
            __file__)
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
        objects.append(NAPObject.fromDict(ob))
    return objects


def saveFile(filename, objects: Iterable[NAPObject]):
    with open(filename, 'w') as fp:
        data = {'Objects': [o.toDict() for o in objects]}
        json.dump(data, fp, indent=2)
