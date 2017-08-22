from PyQt5.QtCore import pyqtSignal
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from generic.filtertreeview import FilterTreeView
from pynap_json.napjsonwrap import NAPInstance, NAPProperty


class ValueItem(QStandardItem):
    def __init__(self, object: NAPInstance, prop: str):
        super(ValueItem, self).__init__()
        self.object = object
        self.prop = prop
        self.updateText()

    def value(self):
        return self.object.value(self.prop)

    def setValue(self, valuestr):
        self.object.setValue(self.prop, valuestr)

    def updateText(self):
        if self.object.hasValue(self.prop):
            self.setText(str(self.object.value(self.prop)))


class KeyItem(QStandardItem):
    def __init__(self, obj, prop):
        super(KeyItem, self).__init__()
        self.obj = obj
        self.prop = prop
        self.setText(prop)

class TypeItem(QStandardItem):
    def __init__(self, obj, prop):
        super(TypeItem, self).__init__()
        self.obj = obj
        self.prop = prop
        self.setText(prop.type())


class PropModel(QStandardItemModel):
    def __init__(self):
        super(PropModel, self).__init__()
        self.__object = None

    def setObjects(self, objects):
        while self.rowCount():
            self.removeRow(0)

        if not objects:
            self.__object = None
            return

        self.__object = objects[0]
        assert isinstance(self.__object, NAPInstance)

        for prop in self.__object.type().properties():
            self.appendRow([
                KeyItem(self.__object, prop.name),
                ValueItem(self.__object, prop.name),
                TypeItem(self.__object, prop)
            ])


class PropPanel(QWidget):
    def __init__(self):
        super(PropPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.__tree = FilterTreeView()
        self.__tree.setModel(PropModel())
        self.layout().addWidget(self.__tree)
        self.__tree.tree().setColumnWidth(0, 200)

    def setObjects(self, objects):
        self.__tree.model().setObjects(objects)
