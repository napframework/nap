from model import *
import nap


def inspectorItemRow(child):
    if isinstance(child, nap.Attribute):
        item = ObjectItem(child)
        return [
            item,
            AttributeValueItem(child),
        ]

    componentItem = ComponentItem(child)
    componentTypeItem = QStandardItem(child.typename())
    componentTypeItem.setEnabled(False)
    items = [
        componentItem,
        componentTypeItem
    ]
    for item in items:
        item.setBackground(QApplication.palette().color(QPalette.Window))
    return items


class InspectorModel(QStandardItemModel):
    modelChanged = pyqtSignal()

    def __init__(self, ctx):
        super(InspectorModel, self).__init__()
        self.__ctx = ctx
        self.__ctx.selectionChanged.connect(self.setEntity)
        self.setHorizontalHeaderLabels(['Name', 'Value'])

    def setEntity(self, obj):
        self.clear()
        if not isinstance(obj, nap.Entity):
            return

        if not obj:
            return

        assert (isinstance(obj, nap.Entity))

        for attrib in obj.attributes():
            self.appendRow(inspectorAttributeRow(attrib))

        for child in obj.children():
            if isinstance(child, nap.Component):
                self.appendRow(inspectorComponentRow(child))

        self.modelChanged.emit()


class BoolEditorCreator(QItemEditorCreatorBase):
    def __init__(self):
        super(BoolEditorCreator, self).__init__()

    def createWidget(self, parent):
        w = QCheckBox(parent)
        # w.setFrame(False)
        return w


class InspectorWidget(QWidget):
    def __init__(self, ctx):
        self.__ctx = ctx
        super(InspectorWidget, self).__init__()
        self.setLayout(QVBoxLayout())

        self.__filter = QLineEdit()
        self.layout().addWidget(self.__filter)

        self.__treeView = QTreeView()
        self.__treeView.header().setStretchLastSection(False)
        self.__treeView.setModel(InspectorModel(ctx))
        self.__treeView.setItemDelegate(QStyledItemDelegate())
        self.__treeView.itemDelegate().setItemEditorFactory(QItemEditorFactory())
        self.__treeView.itemDelegate().itemEditorFactory().registerEditor(QVariant.Bool, BoolEditorCreator())

        self.layout().addWidget(self.__treeView)
        self.__treeView.model().modelChanged.connect(self.__onDataChanged)

    def __onDataChanged(self):
        for i in range(self.__treeView.model().columnCount()):
            self.__treeView.resizeColumnToContents(i)
        self.__treeView.expandAll()
