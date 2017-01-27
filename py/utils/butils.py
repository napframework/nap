import os

def walkDir(d):
    for root, dirs, files in os.walk(d):
        for f in files:
            yield '%s/%s' % (root, f)


def _clamp(n, minVal, maxVal):
    if n < minVal:
        return minVal
    if n > maxVal:
        return maxVal
    return n


def _filter(items, Type):
    for item in items:
        if isinstance(item, Type):
            yield item


def _excludeTypes(items, Types):
    filteredItems = []
    for item in items:
        exclude = False;
        for itemType in Types:
            if isinstance(item, itemType):
                exclude = True
        if not exclude:
            filteredItems.append(item)
    return filteredItems