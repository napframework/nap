#pragma once
#include <QSortFilterProxyModel>

class LeafFilterProxyModel : public QSortFilterProxyModel {
public:

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    /**
     * Traverse up and check if any of the parent nodes matches the filter.
     * @param parent
     * @return
     */
    bool filterAcceptsAnyParent(QModelIndex parent) const;

    bool hasAcceptedChildren(int row, QModelIndex parent) const;
};