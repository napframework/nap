from PyQt5.QtCore import Qt, QTimer, QSettings
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, QTreeView, QAction, QMenu, QApplication, \
    QMessageBox, QFileDialog

import iconstore
import nap
from outline.attributevaluedelegate import AttributeValueDelegate
from outline.model import ObjectItem
from outline.outlinemodel import OutlineModel
from outline.typefilterwidget import TypeFilterWidget
from utils import qtutils


class OutlineWidget(QWidget):
    def __init__(self, ctx, objectName):
        """
        @type ctx: appcontext.AppContext
        """
        self.ctx = ctx
        super(OutlineWidget, self).__init__()

        self.setObjectName(objectName)

        self.setEnabled(False)

        self.__propagateSelection = False
        self.setLayout(QVBoxLayout())

        headerLayout = QHBoxLayout()
        headerLayout.setContentsMargins(0, 0, 0, 0)
        headerLayout.setSpacing(2)

        self.__filterEdit = QLineEdit()
        self.__filterEdit.textEdited.connect(self.__onFilterChanged)
        headerLayout.addWidget(self.__filterEdit)

        self.__typeFilter = TypeFilterWidget()
        self.__typeFilter.filterChanged.connect(self.__onTypeFilterUpdated)
        self.__typeFilter.setTypes([nap.Entity, nap.Component, nap.Attribute])
        headerLayout.addWidget(self.__typeFilter)

        self.layout().addLayout(headerLayout)

        self.__treeView = QTreeView()
        self.__treeView.setSortingEnabled(True)
        self.__treeView.setSelectionMode(QTreeView.ExtendedSelection)
        self.__treeView.setItemDelegateForColumn(2, AttributeValueDelegate())

        self.__outlineModel = OutlineModel(ctx)
        self.__outlineModel.dataChanged.connect(self.onDataChanged)
        self.__filterModel = qtutils.LeafFilterProxyModel()
        self.__filterModel.setItemFilter(self.__itemFilter)
        self.__filterModel.setSourceModel(self.__outlineModel)
        self.__filterModel.setFilterCaseSensitivity(Qt.CaseInsensitive)
        self.__treeView.setModel(self.__filterModel)
        self.__treeView.selectionModel().selectionChanged.connect(
            self.__onSelectionChanged)
        self.__treeView.setContextMenuPolicy(Qt.CustomContextMenu)
        self.__treeView.customContextMenuRequested.connect(
            self.__onCustomContextMenuRequested)
        self.layout().addWidget(self.__treeView)

        self.ctx.applicationClosing.connect(self.onCloseApp)
        self.ctx.connectionChanged.connect(self.onConnectionChanged)
        QTimer.singleShot(0, self.onDataChanged)
        self.__restoreHeaderState()

    def __restoreHeaderState(self):
        s = QSettings()
        headerStateName = 'HeaderState' + self.objectName()
        headerState = s.value(headerStateName)
        if headerState:
            self.__treeView.header().restoreState(headerState)

    def __saveHeaderState(self):
        s = QSettings()
        headerStateName = 'HeaderState' + self.objectName()
        s.setValue(headerStateName, self.__treeView.header().saveState())

    def __onTypeFilterUpdated(self, types):
        self.__filterModel.invalidate()

    def setPropagateSelection(self, b):
        self.__propagateSelection = b

    def expandAll(self):
        self.__treeView.expandAll()

    def setRoot(self, obj):
        """
        @type obj: nap.Object
        """
        self.setEnabled(bool(obj))
        if obj:
            assert (isinstance(obj, nap.Object))

        self.__outlineModel.setRoot(obj)
        # Hide root if necessary
        if not self.__outlineModel.isRootVisible():
            self.__treeView.setRootIndex(self.__treeView.model().index(0, 0))

    def setRootVisible(self, b):
        self.__outlineModel.setRootVisible(b)

    def setFilterTypes(self, types):
        self.__typeFilter.setTypesEnabled(types)

    def __itemFilter(self, item):
        types = self.__typeFilter.enabledTypes()
        assert (isinstance(item, ObjectItem))
        if not types:
            return True
        for t in types:
            if isinstance(item.object(), t):
                return True
        return False

    def onDataChanged(self):
        # self.__treeView.expandAll()
        # for i in reversed(range(self.__treeView.model().columnCount())):
        #     self.__treeView.resizeColumnToContents(i)
        pass

    def onCloseApp(self):
        self.__saveHeaderState()

    def onConnectionChanged(self, *args):
        # qtutils.restoreExpanded(self.__treeView, 'OutlineExpandedState')
        pass

    def __onFilterChanged(self):
        self.__filterModel.setFilterRegExp(self.__filterEdit.text())

    def __selectedItem(self):
        """
        @rtype: ObjectItem
        """
        for item in self.__selectedItems():
            return item

    def __selectedItems(self):
        if not self.__treeView.selectionModel().hasSelection():
            return

        for idx in self.__treeView.selectionModel().selectedRows():
            idx = self.__filterModel.mapToSource(idx)
            item = self.__outlineModel.itemFromIndex(idx)
            yield item

    def __selectedObject(self):
        """
        @rtype: nap.Object
        """
        for item in self.__selectedItems():
            return item.object()

    def __selectedObjects(self):
        for item in self.__selectedItems():
            yield item.object()

    def __onSelectionChanged(self):
        if self.__propagateSelection:
            self.ctx.setSelection(self.__selectedObject())

    # def __createComponentActions(self, parentObj, menu):
    #     for compType in self.ctx.core().componentTypes():
    #         print(compType)
    #         action = QAction(iconstore.icon('brick_add'), compType, menu)
    #         action.triggered[()].connect(lambda compType=compType: self.ctx.core().addChild(parentObj, compType))
    #         yield action

    @staticmethod
    def __iconAction(menu, text, icon, callback):
        a = QAction(iconstore.icon(icon), text, menu)
        a.triggered.connect(callback)
        menu.addAction(a)

    def __onCustomContextMenuRequested(self, pos):
        selectedObject = self.__selectedObject()
        if not selectedObject:
            return

        menu = QMenu()

        self.__iconAction(menu, 'Expand All', 'toggle-expand',
                          self.__onExpandSelection)
        self.__iconAction(menu, 'Collapse All', 'toggle',
                          self.__onCollapseSelection)

        menu.addSeparator()

        editor = self.ctx.hasEditorFor(selectedObject)
        if editor:
            self.__iconAction(menu, 'Edit', 'brick', self.__onShowEditor)

        if isinstance(selectedObject, nap.Entity):
            self.__iconAction(menu, 'Add Child', 'add', self.__onAddChild)

            addCompMenu = menu.addMenu(iconstore.icon('brick_add'),
                                       'Add Component...')
            self.ctx.createObjectActions(selectedObject,
                                         self.ctx.core().componentTypes(),
                                         addCompMenu)

        menu.addSeparator()

        self.__iconAction(menu, 'Cut', 'cut', self.__onCutObjects)
        self.__iconAction(menu, 'Copy', 'copy', self.__onCopyObjects)
        self.__iconAction(menu, 'Paste', 'paste', self.__onPasteObjects)
        self.__iconAction(menu, 'Remove', 'delete', self.__onRemoveObjects)

        menu.addSeparator()

        self.__iconAction(menu, 'Import...', 'folder_page',
                          self.__onImportObject)
        self.__iconAction(menu, 'Reference...', 'page_link',
                          self.__onReferenceObject)
        self.__iconAction(menu, 'Export...', 'disk', self.__onExportSelected)

        menu.exec_(self.__treeView.viewport().mapToGlobal(pos))

    def __onShowEditor(self):
        self.ctx.requestEditorFor(self.__selectedObject())

    def __onAddChild(self):
        parentObj = self.__selectedObject()
        assert parentObj
        if parentObj:
            self.ctx.core().addEntity(parentObj)

    def __onCutObjects(self):
        self.__onCopyObjects()
        self.__onRemoveObjects()

    def __onCopyObjects(self):
        self.ctx.core().copyObjectTree(self.__selectedObject())

    def __onPasteObjects(self):
        jsonStr = str(QApplication.clipboard().text())
        self.ctx.core().pasteObjectTree(self.__selectedObject(), jsonStr)

    def __onRemoveObjects(self):
        print('Remove objects %s' % list(self.__selectedObjects()))
        self.ctx.core().removeObjects(self.__selectedObjects())

    def __onReferenceObject(self):
        QMessageBox.information(self, 'Sorry', 'Not implemented yet')

    def __onImportObject(self):
        parentObj = self.__selectedObject()
        filename = QFileDialog.getOpenFileName(self,
                                               'Select object file to import',
                                               self.ctx.lastFileDir(),
                                               self.ctx.napFileFilter())
        filename = filename[0]  # First was filename, second is filter
        if not filename:
            return

        self.ctx.importObject(parentObj, filename)

    def __onExportSelected(self):
        obj = self.__selectedObject()
        filename = QFileDialog.getSaveFileName(self, 'Select destination file',
                                               self.ctx.lastFileDir(),
                                               self.ctx.napFileFilter())
        filename = filename[0]  # First was filename, second is filter

        if not filename:
            return
        self.ctx.exportObject(obj, filename)

    def __onExpandSelection(self):
        item = self.__selectedItem()
        if not item:
            return
        qtutils.expandChildren(self.__treeView,
                               self.__filterModel.mapFromSource(item.index()))

    def __onCollapseSelection(self):
        item = self.__selectedItem()
        if not item:
            return
        qtutils.expandChildren(self.__treeView,
                               self.__filterModel.mapFromSource(item.index()),
                               False)