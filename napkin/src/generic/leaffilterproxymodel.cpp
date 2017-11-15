#include "leaffilterproxymodel.h"

bool LeafFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
        return true;

    return acceptsAnyChild(sourceRow, sourceParent);
}

bool LeafFilterProxyModel::acceptsAnyChild(int sourceRow, QModelIndex sourceParent) const {
    auto childIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    for (int r = 0, len = sourceModel()->rowCount(childIndex); r < len; r++) {
        if (filterAcceptsRow(r, childIndex))
            return true;
    }
    return false;
}
