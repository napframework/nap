from itertools import chain
from typing import Iterable

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

from generic.filtertreeview import FilterTreeView
from pynap_json.napjsonwrap import *


class ObjectItem(QStandardItem):
    def __init__(self, obj:NAPObject):
        super(ObjectItem, self).__init__()
        self.object = obj
        self.object.changed.connect(self.refresh)
        self.refresh()

        for ob in chain(self.object.components(), self.object.children()):
            self.appendRow([
                ObjectItem(ob),
                ObjectTypeItem(ob),
            ])

    def refresh(self):
        self.setText(self.object.name())

    def setData(self, value, role):
        if role == Qt.EditRole:
            self.object.setName(value)
        super(ObjectItem, self).setData(value, role)
            
class ComponentItem(QStandardItem):
    def __init__(self, comp:NAPComponent):
        super(ComponentItem, self).__init__()
        self.component = comp
        self.setText(comp.name())


class ObjectTypeItem(QStandardItem):
    def __init__(self, obj):
        super(ObjectTypeItem, self).__init__()
        self.object = obj
        self.setText(self.object.typeName())


class ObjectModel(QStandardItemModel):
    def __init__(self):
        super(ObjectModel, self).__init__()
        self.setHorizontalHeaderLabels(['mID', 'Type'])

    def setObjects(self, objects: Iterable[NAPObject]):
        while self.rowCount():
            self.removeRow(0)
        for ob in objects:
            self.appendRow([
                ObjectItem(ob),
                ObjectTypeItem(ob),
            ])

    def items(self) -> Iterable[ObjectItem]:
        for row in range(self.rowCount()):
            yield self.item(row)

    def objects(self) -> Iterable[NAPObject]:
        return [item.object for item in self.items()]


class OutlinePanel(QWidget):
    selectionChanged = pyqtSignal(list)

    def __init__(self):
        super(OutlinePanel, self).__init__()
        self.setLayout(QVBoxLayout())

        self.__tree = FilterTreeView()
        self.__tree.setModel(ObjectModel())
        self.__tree.tree().setColumnWidth(0, 300)
        self.__tree.selectionModel().selectionChanged.connect(
            self.__onSelectionChanged)
        # self.__tree.tree().setSortingEnabled(True)
        self.__tree.model().rowsInserted.connect(self.__onRowsInserted)
        self.layout().addWidget(self.__tree)

    def __onSelectionChanged(self):
        objects = [m.object for m in self.__tree.selectedItems()]
        self.selectionChanged.emit(objects)

    def __onRowsInserted(self, parent, first, last):
        self.__tree.expandAll()

    def setObjects(self, objects: Iterable[NAPObject]):
        self.model().setObjects(objects)

    def model(self) -> ObjectModel:
        return self.__tree.model()

    def setModel(self, model: ObjectModel):
        self.__tree.setModel(model)
