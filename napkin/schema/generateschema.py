import json
from collections import OrderedDict

import jsonschema as jsonschema
import nap
import os

from pynap.naputils import fullCPPTypename, napClasses

__NO_ADD_PROPS = ('additionalProperties', False)
__TYPE_OBJECT = ('type', 'object')

__PY2JSON_TYPEMAP = {
    bool: 'boolean',
    int: 'integer',
    str: 'string',
    float: 'number',
    list: 'array',
    None: 'null',
}


def propType(naptype, prop):
    try:
        obj = naptype()
        t = type(getattr(obj, prop))
        if not t in __PY2JSON_TYPEMAP:
            print('How to define type in schema? %s' % t)
            return None
        return __PY2JSON_TYPEMAP[t]
    except Exception as e:
        print("While getting property type: %s" % e)
        return None


def props(naptype):
    dic = OrderedDict([
        ('Type', OrderedDict([
            ('type', 'string'),
            # ('enum', [fullCPPTypename(naptype)])
            ('pattern', fullCPPTypename(naptype))
        ])),
        ('mID', {'type': 'string'})])

    for k in dir(naptype):
        if k.startswith('__'): continue
        if k == 'mID': continue
        typestr = propType(naptype, k)
        if typestr:
            dic[k] = {'type': typestr}  # TODO: Property type
        else:
            dic[k] = {}
    return dic


def resourceDef(naptype):
    return OrderedDict([
        __TYPE_OBJECT, __NO_ADD_PROPS,
        ('required', ['Type']),
        ('properties', props(naptype))
    ])


def resourceDefinitions(naptypes):
    dic = OrderedDict()
    for t in naptypes:
        dic[fullCPPTypename(t)] = resourceDef(t)
    return dic


def resourceRef(naptype):
    return {'$ref': '#/definitions/%s' % fullCPPTypename(naptype)}


def writeSchema(filename):
    # types = [nap.RenderWindow, nap.MemoryTexture2D, nap.Entity]
    naptypes = list(napClasses(nap.RTTIObject))
    types = naptypes

    dic = OrderedDict([
        ('$schema', 'http://json-schema.org/draft-04/schema#'),
        ('description', 'NAP JSON Schema'),
        __TYPE_OBJECT, __NO_ADD_PROPS,
        ('required', ['Objects']),
        ('properties', OrderedDict([
            ('Objects', OrderedDict([
                ('type', 'array'),
                ('items', {'anyOf': [resourceRef(t) for t in types]})
            ])),
            # ('entities', {})
        ])),
        ('definitions', resourceDefinitions(types))
    ])

    with open(filename, 'w') as fp:
        json.dump(dic, fp, indent=2)
        print('Wrote: %s' % os.path.abspath(filename))


def testSchema(schemaFile, testFile):
    print('Validating "%s" against "%s"' % (os.path.abspath(testFile), os.path.abspath(schemaFile)))
    with open(schemaFile, 'r') as fp:
        schema = json.load(fp)
    with open(testFile, 'r') as fp:
        testdata = json.load(fp)

    jsonschema.validate(testdata, schema)

    print('Validation OK')


if __name__ == '__main__':
    file_test = 'test_schema_valid.nap.json'
    file_schema = 'napschema.json'

    writeSchema(file_schema)
    testSchema(file_schema, file_test)
