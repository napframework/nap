#pragma once
#include <QSortFilterProxyModel>

class LeafFilterProxyModel : public QSortFilterProxyModel {
public:

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool acceptsAnyChild(int sourceRow, QModelIndex sourceParent) const;

};