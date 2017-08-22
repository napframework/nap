from PyQt5.QtCore import pyqtSignal
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from generic.filtertreeview import FilterTreeView


class ObjectItem(QStandardItem):
    def __init__(self, obj):
        super(ObjectItem, self).__init__()
        self.object = obj
        self.setText(self.object.name())

class ObjectTypeItem(QStandardItem):
    def __init__(self, obj):
        super(ObjectTypeItem, self).__init__()
        self.object = obj
        self.setText(self.object.typeName())


class ObjectModel(QStandardItemModel):
    def __init__(self):
        super(ObjectModel, self).__init__()
        self.setHorizontalHeaderLabels(['mID', 'Type'])


    def setObjects(self, objects):
        while self.rowCount():
            self.removeRow(0)
        for ob in objects:
            self.appendRow([
                ObjectItem(ob),
                ObjectTypeItem(ob)
            ])


class ObjectPanel(QWidget):

    selectionChanged = pyqtSignal(list)


    def __init__(self):
        super(ObjectPanel, self).__init__()
        self.setLayout(QVBoxLayout())

        self.__tree = FilterTreeView()
        self.__tree.setModel(ObjectModel())
        self.__tree.tree().setColumnWidth(0, 300)
        self.__tree.selectionModel().selectionChanged.connect(self.__onSelectionChanged)
        # self.__tree.tree().setSortingEnabled(True)

        self.layout().addWidget(self.__tree)

    def __onSelectionChanged(self):
        objects = [m.object for m in self.__tree.selectedItems()]
        self.selectionChanged.emit(objects)

    def model(self) -> ObjectModel:
        return self.__tree.model()

    def setModel(self, model: ObjectModel):
        self.__tree.setModel(model)
