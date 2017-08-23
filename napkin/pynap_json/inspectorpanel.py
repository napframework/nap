from PyQt5.QtCore import pyqtSignal
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from generic.filtertreeview import FilterTreeView
from pynap_json.constants import PROP_COMPONENTS, PROP_CHILDREN
from pynap_json.napjsonwrap import NAPObject, NAPProperty


class ValueItem(QStandardItem):
    def __init__(self, object: NAPObject, prop: str):
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
        self.setHorizontalHeaderLabels(['Property', 'Value', 'Type'])

    def setObjects(self, objects):
        while self.rowCount():
            self.removeRow(0)

        if not objects:
            self.__object = None
            return

        self.__object = objects[0]
        assert isinstance(self.__object, NAPObject)

        objtype = self.__object.type()
        if not objtype:
            return

        for prop in objtype.properties():
            if prop.name == PROP_COMPONENTS: continue
            if prop.name == PROP_CHILDREN: continue

            self.appendRow([
                KeyItem(self.__object, prop.name),
                ValueItem(self.__object, prop.name),
                TypeItem(self.__object, prop)
            ])


class InspectorPanel(QWidget):
    def __init__(self):
        super(InspectorPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.__tree = FilterTreeView()
        self.__tree.setModel(PropModel())
        self.layout().addWidget(self.__tree)
        self.__tree.tree().setColumnWidth(0, 150)
        self.__tree.tree().setColumnWidth(1, 200)

    def setObjects(self, objects):
        self.__tree.model().setObjects(objects)
        self.__tree.expandAll()
