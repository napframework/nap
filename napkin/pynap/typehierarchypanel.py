from collections import OrderedDict

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

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

class TypeHierarchyPanel(QWidget):
    def __init__(self):
        super(TypeHierarchyPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.__downstreamModel = TypeByRootModel()
        self.__upstreamModel = TypeByLeafModel()

        self.__modelRadioButtons = QWidget()
        self.__modelRadioButtons.setLayout(QHBoxLayout())
        self.__modelRadioButtons.layout().setContentsMargins(0, 0, 0, 0)
        self.__modelRadioButtons.layout().addStretch(1)
        self.layout().addWidget(self.__modelRadioButtons)

        self.__modelRadioGrp = QButtonGroup()
        self.__modelRadioGrp.buttonToggled.connect(self.__refresh)
        self.__rbDownstream = QRadioButton('Downstream')
        self.__rbDownstream.setChecked(True)
        self.__modelRadioGrp.addButton(self.__rbDownstream)
        self.__modelRadioButtons.layout().addWidget(self.__rbDownstream)

        self.__rbUpstream = QRadioButton('Upstream')
        self.__modelRadioGrp.addButton(self.__rbUpstream)
        self.__modelRadioButtons.layout().addWidget(self.__rbUpstream)

        self.__treeView = QTreeView()
        self.__treeView.setHeaderHidden(True)
        self.layout().addWidget(self.__treeView)

        self.__refresh()

    def __refresh(self):
        if self.__rbDownstream.isChecked():
            self.__treeView.setModel(self.__downstreamModel)
            self.__treeView.expandAll()
        elif self.__rbUpstream.isChecked():
            self.__treeView.setModel(self.__upstreamModel)
