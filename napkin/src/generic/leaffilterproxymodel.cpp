#include "leaffilterproxymodel.h"

using namespace napkin;

LeafFilterProxyModel::LeafFilterProxyModel() : QSortFilterProxyModel() {}

bool napkin::LeafFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
		return true;

	// Run through any exempted indexes
	for (int col = 0, len=sourceModel()->columnCount(sourceParent); col < len; col++)
	{
		const auto sourceIndex = sourceParent.child(sourceRow, col);
		if (mExemptions.contains(sourceIndex))
			return true;
	}

	return acceptsAnyChild(sourceRow, sourceParent);
}

bool napkin::LeafFilterProxyModel::acceptsAnyChild(int sourceRow, QModelIndex sourceParent) const
{
	QModelIndex childIndex = sourceModel()->index(sourceRow, 0, sourceParent);

	for (int r = 0, len = sourceModel()->rowCount(childIndex); r < len; r++)
	{
		if (filterAcceptsRow(r, childIndex))
			return true;
	}
	return false;
}

void LeafFilterProxyModel::exemptSourceIndex(QModelIndex sourceIndex)
{
	mExemptions << sourceIndex;
	invalidateFilter();
}

void LeafFilterProxyModel::clearExemptions()
{
	mExemptions.clear();
	invalidateFilter();
}


