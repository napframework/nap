#pragma once
#include <QSortFilterProxyModel>

class LeafFilterProxyModel : public QSortFilterProxyModel {
public:

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (!source_parent.isValid())
            return true;

        if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
            return true;

        return hasAcceptedChildren(source_row, source_parent);
    }

private:
    /**
     * Traverse up and check if any of the parent nodes matches the filter.
     * @param parent
     * @return
     */
    bool filterAcceptsAnyParent(QModelIndex parent) const {
        while (parent.isValid()) {
            if (QSortFilterProxyModel::filterAcceptsRow(parent.row(), parent.parent()))
                return true;
            parent = parent.parent();
        }
        return false;
    }

    bool hasAcceptedChildren(int row, QModelIndex parent) const {
        auto model = sourceModel();
        auto sourceIndex = model->index(row, 0, parent);

        for (int r = 0, len = model->rowCount(sourceIndex); r < len; r++) {
            if (filterAcceptsRow(row, sourceIndex))
                return true;
        }
        return false;
    }
};