from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
import iconstore
import napclient

def inspectorAttributeRow(attrib):
    assert(isinstance(attrib, napclient.Attribute))
    return [
        ObjectItem(attrib),
        AttributeValueItem(attrib),
    ]

def inspectorComponentRow(comp):
    assert(isinstance(comp, napclient.Component))
    componentItem = ComponentItem(comp)
    componentTypeItem = QStandardItem(comp.typename())
    componentTypeItem.setEnabled(False)
    items = [
        componentItem,
        componentTypeItem
    ]
    for item in items:
        item.setBackground(QApplication.palette().color(QPalette.Window))
    return items



class ObjectItem(QStandardItem):
    """ This item wraps an Object.
    """

    ObjectType = napclient.NObject

    def __init__(self, obj):
        """
        @type obj: napclient.NObject
        """
        assert(isinstance(obj, self.ObjectType))
        super(ObjectItem, self).__init__()
        self.__obj = obj
        self.__obj.nameChanged.connect(self.__onNameChanged)
        self.__obj.childAdded.connect(self.onChildAdded)
        self.__obj.childRemoved.connect(self.onChildRemoved)
        self.setEditable(self.__obj.isEditable())
        self.setText(obj.name())
        self.setToolTip('%s (%s)' % (obj.name(), obj.typename()))
        self.setData(obj.ptr(), Qt.UserRole)
        self.setIcon(iconstore.iconFor(obj))

        for obj in obj.children():
            self.onChildAdded(obj)

    def __onNameChanged(self, name):
        self.setText(name)

    def setData(self, variant, role=None):
        if role == Qt.EditRole:
            self.__obj.setName(str(variant))
        else:
            super(ObjectItem, self).setData(variant, role)

    def object(self):
        return self.__obj

    def findRow(self, obj):
        for row in range(self.rowCount()):
            item = self.child(row)
            if item.object() == obj:
                return row
        return -1

    def onChildAdded(self, obj):
        self.appendRow(createItemRow(obj))

    def onChildRemoved(self, obj):
        self.removeRow(self.findRow(obj))


class ComponentItem(ObjectItem):

    ObjectType = napclient.Component

    def __init__(self, comp):
        """
        @type attr: napclient.Component
        """
        super(ComponentItem, self).__init__(comp)



class EntityItem(ObjectItem):

    ObjectType = napclient.Entity

    def __init__(self, obj):
        """
        @type obj: napclient.Entity
        """
        super(EntityItem, self).__init__(obj)




class AttributeValueItem(QStandardItem):
    def __init__(self, attrib):
        super(AttributeValueItem, self).__init__()
        self.__attrib = attrib
        self.setText("BLAAT")
        if isinstance(attrib.value(), bool):
            self.setData(QVariant(bool(attrib.value())), Qt.DisplayRole)
        else:
            self.setText(str(attrib.value() or ''))

        self.setEditable(self.__attrib.isEditable())
        # self.setBackground(Qt.Dis)

    def setData(self, variant, role=None):
        if role == Qt.EditRole:
            self.__attrib.setValue(variant.toPyObject())
        else:
            QStandardItem.setData(self, variant, role)

class ObjectTypeItem(QStandardItem):
    def __init__(self, obj):
        super(ObjectTypeItem, self).__init__()
        self.setEditable(False)
        if isinstance(obj, napclient.Attribute):
            self.setText(obj.valueType())
        else:
            self.setText(obj.typename())

def createItem(obj):
    for itemType in (EntityItem, ComponentItem):
        if isinstance(obj, itemType.ObjectType):
            return itemType(obj)
    return ObjectItem(obj)


def createItemRow(obj):
    objectItem = createItem(obj)
    typeItem = ObjectTypeItem(obj)
    valueItem = None
    if isinstance(obj, napclient.Attribute):
        valueItem = AttributeValueItem(obj)

    return [
        objectItem,
        typeItem,
        valueItem,
    ]