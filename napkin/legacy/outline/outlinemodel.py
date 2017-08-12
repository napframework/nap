from PyQt5.QtGui import QStandardItemModel

from outline.model import createItemRow


class OutlineModel(QStandardItemModel):
    def __init__(self, ctx):
        """
        @type ctx: appcontext.AppContext
        """
        self.ctx = ctx
        super(OutlineModel, self).__init__()

        self.__isRootVisible = True
        self.setHorizontalHeaderLabels(['Name', 'Type', 'Value'])

    def root(self):
        """
        @rtype napclient.Object
        """
        return self.__object

    def setRoot(self, obj):
        """ Replace the data in this model with the specified object as root
        @type obj: nap.Object
        """
        # self.__object = obj
        self.removeRows(0, self.rowCount())
        if obj:
            itemRow = createItemRow(obj)
            self.appendRow(itemRow)
            return itemRow[0].index()

    def isRootVisible(self):
        return self.__isRootVisible

    def setRootVisible(self, b):
        self.__isRootVisible = b