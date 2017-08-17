from functools import partial

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

import nap

from napkin.generic.filtertreeview import FilterTreeView


class KeyItem(QStandardItem):
    def __init__(self, text):
        super(KeyItem, self).__init__()
        self.setEditable(False)
        self.setText(text)

class ValueItem(QStandardItem):

    def __init__(self, obj, attrib):
        super(ValueItem, self).__init__()
        self.obj = obj
        self.attrib = attrib
        self.attribtype = type(self.value())
        self.setText(str(self.value()))

    def value(self):
        return getattr(self.obj, self.attrib)

    def setValue(self, valuestr):
        try:
            setattr(self.obj, self.attrib, self.attribtype(valuestr))
        except ValueError as e:
            print(e)

    def setData(self, value, role):
        if role == Qt.EditRole:
            self.setValue(value)
            self.setText(str(self.value()))
        else:
            super(ValueItem, self).setData(value, role)



class PropertyModel(QStandardItemModel):
    def __init__(self):
        super(PropertyModel, self).__init__()
        self.obj = None
        self.setHorizontalHeaderLabels(['Name', 'Value'])

    def setObject(self, obj):
        self.removeRows(0, self.rowCount())
        self.obj = obj

        for k in dir(self.obj):
            if k.startswith('__'): continue

            self.appendRow([
                QStandardItem(k),
                ValueItem(self.obj, k),
            ])


class PropertyPanel(QWidget):
    def __init__(self):
        super(PropertyPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.layout().setSpacing(0)

        self.__treeView = FilterTreeView()

        self.__model = PropertyModel()
        self.__treeView.setModel(self.__model)
        self.layout().addWidget(self.__treeView)


    def setItems(self, items):
        for item in items:
            self.__model.setObject(item)
            return
        # self.__model.setObject(next(items, None))
