from PyQt5.QtCore import QSortFilterProxyModel


class LeafFilterProxyModel(QSortFilterProxyModel):
    """ Class to override the following behaviour:
            If a parent item doesn't match the filter,
            none of its children will be shown.

        This Model matches items which are descendants
        or ascendants of matching items.
    """

    def __init__(self):
        super(LeafFilterProxyModel, self).__init__()
        self.__itemFilter = None

    def setItemFilter(self, filter):
        self.__itemFilter = filter

    def filterAcceptsRow(self, row, parentIndex):
        """ Overriding the parent function """
        if not parentIndex.isValid():
            return True

        if self.__itemFilter:
            source_index = self.sourceModel().index(row, 0, parentIndex)
            item = self.sourceModel().itemFromIndex(source_index)
            if item and not self.__itemFilter(item):
                return False

        # Check if the current row matches
        if super(LeafFilterProxyModel, self).filterAcceptsRow(row, parentIndex):
            return True

        # Finally, check if any of the children match
        return self.hasAcceptedChildren(row, parentIndex)

    def filterAcceptsAnyParent(self, parent):
        """ Traverse to the root node and check if any of the
            ancestors match the filter
        """
        while parent.isValid():
            if self.filterAccepsRowItself(parent.row(), parent.parent()):
                return True
            parent = parent.parent()
        return False

    def hasAcceptedChildren(self, row_num, parent):
        """ Starting from the current node as root, traverse all
            the descendants and _sandbox if any of the children match
        """
        model = self.sourceModel()
        source_index = model.index(row_num, 0, parent)

        children_count = model.rowCount(source_index)
        for i in range(children_count):
            if self.filterAcceptsRow(i, source_index):
                return True
        return False