from collections import OrderedDict
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from napkin.pynap import *
from napkin.pynap import naputils


class TypeByLeafItem(QStandardItem):
    def __init__(self, cls):
        super(TypeByLeafItem, self).__init__()
        self.cls = cls
        self.setText(cls.__name__)

        for c in cls.__bases__:
            self.appendRow(TypeByLeafItem(c))


class TypeByLeafModel(QStandardItemModel):
    def __init__(self):
        super(TypeByLeafModel, self).__init__()

        for c in naputils.napClasses():
            self.appendRow(TypeByLeafItem(c))


class TypeItem(QStandardItem):
    def __init__(self, cls):
        super(TypeItem, self).__init__()
        self.__items = {}
        self.setText(cls.__name__)
        self.cls = cls

    def addItem(self, derivedList):
        cls = derivedList.pop(0)
        item = self.getItem(cls)
        if derivedList:
            item.addItem(derivedList)

    def getItem(self, cls):
        if cls not in self.__items:
            item = TypeItem(cls)
            self.appendRow(item)
            self.__items[cls] = item
        return self.__items[cls]


class TypeByRootModel(QStandardItemModel):
    def __init__(self):
        super(TypeByRootModel, self).__init__()
        self.__items = OrderedDict()

        for derived in ([c] + naputils.baseClassList(c) for c in naputils.napClasses()):
            derived = list(reversed(derived))
            self.addItem(derived)

    def addItem(self, derivedList):
        cls = derivedList.pop(0)
        self.getItem(cls).addItem(derivedList)

    def getItem(self, cls):
        if cls not in self.__items:
            item = TypeItem(cls)
            self.appendRow(item)
            self.__items[cls] = item
        return self.__items[cls]