import sys
from collections import OrderedDict

import os
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

# TODO: Dirty hack to get things going right now
from napkin.generic.qbasewindow import QBaseWindow

p = '%s/../../lib/Clang-Debug-x86_64' % os.path.dirname(__file__)
sys.path.append(p)

import nap


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

        for c in napClasses():
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

        for derived in ([c] + baseClassList(c) for c in napClasses()):
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


def napClasses():
    for k, v in nap.__dict__.items():
        if not isinstance(v, type): continue
        yield v


def baseClassList(cls, outlist=None):
    if not outlist:
        outlist = []
    base = next((c for c in cls.__bases__), None)
    if not base:
        return outlist
    outlist.append(base)
    baseClassList(base, outlist)
    return outlist


class TypeHierarchyPanel(QWidget):
    def __init__(self):
        super(TypeHierarchyPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.__downstreamModel = TypeByRootModel()
        self.__upstreamModel = TypeByLeafModel()

        self.__radioGrp = QHBoxLayout()
        self.__radioGrp.setContentsMargins(0, 0, 0, 0)
        self.layout().addChildLayout(self.__radioGrp)

        self.__rbDownstream = QRadioButton('Downstream')
        self.__rbDownstream.setChecked(True)
        self.__radioGrp.addWidget(self.__rbDownstream)
        self.__rbUpstream = QRadioButton('Upstream')
        self.__radioGrp.addWidget(self.__rbUpstream)

        self.__treeView = QTreeView()
        self.__treeView.setHeaderHidden(True)
        self.layout().addWidget(self.__treeView)
        self.__refresh()

    def __refresh(self):
        if self.__rbDownstream.isChecked():
            self.__treeView.setModel(self.__downstreamModel)
        elif self.__rbUpstream.isChecked():
            self.__treeView.setModel(self.__upstreamModel)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = QBaseWindow()

    win.addDock('Type Hierarchy', TypeHierarchyPanel())

    win.show()
    app.exec()
