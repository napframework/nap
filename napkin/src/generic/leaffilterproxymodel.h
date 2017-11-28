#pragma once
#include <QSortFilterProxyModel>

/**
 * A variation on QSortFilterProxyModel.
 * When filtering, this model includes matches in a deeper levels of the hierarchy.
 */
class LeafFilterProxyModel : public QSortFilterProxyModel
{
public:
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
	bool acceptsAnyChild(int sourceRow, QModelIndex sourceParent) const;
};