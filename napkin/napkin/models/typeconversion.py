# conversion between python types
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
