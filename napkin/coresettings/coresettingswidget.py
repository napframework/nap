from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import appcontext
import iconstore
import nap
from utils.butils import allSubClasses


class ModuleModel(QStandardItemModel):
    def __init__(self, ctx, modInfo):
        """
        @type ctx: appcontext.AppContext
        @type modInfo: core_native.ModuleInfo
        """
        super(ModuleModel, self).__init__()
        self.ctx = ctx

        globalItem = self._item('Global', 'world')

        modulesItem = self._item('Modules', 'wooden-box')
        self.appendRow(modulesItem)
        for mod in modInfo.modules():
            modItem = self._item(mod.name(), 'box')
            modItem.appendRow(self._item(mod.filename(), 'document'))
            modulesItem.appendRow(modItem)

        self.__addTypesItem(globalItem, ctx.core().componentTypes)
        self.__addTypesItem(globalItem, ctx.core().operatorTypes)
        self.__addTypesItem(globalItem, ctx.core().dataTypes)

        # typeIcon = iconstore.icon('type')

        # typeHierarchyItem = QStandardItem(typeIcon, 'baseTypes')
        # types = modInfo.types()
        # for tp in types:
        #     typeItem = QStandardItem(typeIcon, tp.name())
        #     typeHierarchyItem.appendRow(typeItem)
        #     parentItem = typeItem
        #     for baseType in tp.baseTypes():
        #         baseTypeItem = QStandardItem(typeIcon, baseType)
        #         parentItem.appendRow(baseTypeItem)
        #         parentItem = baseTypeItem
        # self.appendRow(typeHierarchyItem)

        self.appendRow(globalItem)

    def __addTypesItem(self, parent, getter):
        item = self._item(getter.__name__, 'folder_brick')
        for t in getter():
            item.appendRow(self._item(t, 'brick'))
        parent.appendRow(item)

    def _nameForDict(self, dic):
        """
        @type dic: dict
        """
        if 'name' in dic:
            return dic['name']
        for val in dic.values():
            if isinstance(val, str):
                return val
        return '<dict>'

    def _item(self, text, icon=None):
        if isinstance(text, dict):
            text = ModuleModel.__nameForDict(text)

        item = QStandardItem(str(text))
        item.setEditable(False)
        if icon:
            item.setIcon(iconstore.icon(icon))
        return item

    def _addKeyValRow(self, parent, key, val):
        keyItem = self._item(key, 'ui-text-field-format')
        valItem = self._item(val)
        parent.appendRow([keyItem, valItem])


class CoreSettingsWidgetBase(QWidget):
    def __init__(self):
        super(CoreSettingsWidgetBase, self).__init__()

class PyCoreSettingsWidget(CoreSettingsWidgetBase):
    CORE_TYPE = nap.Core

    def __init__(self, ctx):
        """
        @type ctx: appcontext.AppContext
        """
        super(PyCoreSettingsWidget, self).__init__()
        self.ctx = ctx
        self.core = ctx.core()


# class NAPCoreSettingsWidget(CoreSettingsWidgetBase):
#     CORE_TYPE = NAPCore
#
#     def __init__(self, ctx):
#         """
#         @type ctx: appcontext.AppContext
#         """
#         super(NAPCoreSettingsWidget, self).__init__()
#         self.ctx = ctx
#         self.core = ctx.core()
#         self.setLayout(QVBoxLayout())
#
#         connectionLayout = QHBoxLayout()
#         connectionLayout.setContentsMargins(0, 0, 0, 0)
#         self.tfHost = QLineEdit()
#         connectionLayout.addWidget(self.tfHost)
#         self.btConnect = QPushButton('Refresh')
#         connectionLayout.addWidget(self.btConnect)
#         self.btConnect.clicked.connect(self.onConnect)
#         self.lbMessage = QLabel()
#         connectionLayout.addWidget(self.lbMessage)
#         self.layout().addLayout(connectionLayout)
#
#         ctx.coreChanged.connect(self.onCoreChanged)
#
#     def onConnect(self):
#         self.ctx.connect(self.tfHost.text())
#
#     def onCoreChanged(self, connected, hostname, message):
#         self.tfHost.setText(hostname)
#         self.lbMessage.setText(message)


class CoreSettingsWidget(QWidget):
    coreTypeWidgetTypes = [PyCoreSettingsWidget]

    def __init__(self, ctx):
        """
        @type ctx: appcontext.AppContext
        """
        super(CoreSettingsWidget, self).__init__()
        self.ctx = ctx
        self.ctx.coreChanged.connect(self.__onCoreChanged)

        self.setMinimumHeight(10)

        self.setLayout(QVBoxLayout())

        self.cbCoreType = QComboBox()
        for coreType in appcontext.CORE_TYPES:
            self.cbCoreType.addItem(coreType.__name__, coreType)
        self.cbCoreType.currentIndexChanged.connect(self.__onCoreTypeChanged)
        self.layout().addWidget(self.cbCoreType)

        self.coreTypeWidget = None
        self.coreTypeWidgetHolder = QWidget()
        self.coreTypeWidgetHolder.setLayout(QVBoxLayout())
        self.layout().addWidget(self.coreTypeWidgetHolder)

        self.__treeView = QTreeView()
        self.__treeView.setHeaderHidden(True)
        self.layout().addWidget(self.__treeView)

        ctx.core().moduleInfoChanged.connect(self.__onModuleInfoChanged)

    def __replaceCoreTypeWidget(self, newWidget):
        # Remove old one
        if self.coreTypeWidget:
            self.coreTypeWidgetHolder.layout().removeWidget(self.coreTypeWidget)
            del self.coreTypeWidget
            self.coreTypeWidget = None
        # Add new one
        self.coreTypeWidget = newWidget
        self.coreTypeWidgetHolder.layout().addWidget(self.coreTypeWidget)

    def createSettingsWidget(self, coreType):
        for widgetClass in allSubClasses(CoreSettingsWidgetBase):
            if widgetClass.CORE_TYPE == coreType:
                return widgetClass(self.ctx)

    def __onCoreTypeChanged(self, index):
        coreType = self.cbCoreType.itemData(index)
        settingsWidget = self.createSettingsWidget(coreType)
        self.__replaceCoreTypeWidget(settingsWidget)

    def __onCoreChanged(self, core):
        self.__onModuleInfoChanged(core.moduleInfo())

    def __onModuleInfoChanged(self, info):
        self.__treeView.setModel(ModuleModel(self.ctx, info))
        self.__treeView.expandAll()
