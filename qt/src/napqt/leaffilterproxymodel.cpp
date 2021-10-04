/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "leaffilterproxymodel.h"

using namespace nap::qt;

LeafFilterProxyModel::LeafFilterProxyModel() : QSortFilterProxyModel() {}

bool LeafFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	if (isExempt(sourceRow, sourceParent))
		return true;

	for (const auto& extraFilter : mExtraFilters)
		if (!extraFilter(*this, sourceRow, sourceParent))
			return false;

	if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
		return true;


	return acceptsAnyChild(sourceRow, sourceParent);
}

bool LeafFilterProxyModel::acceptsAnyChild(int sourceRow, QModelIndex sourceParent) const
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

void LeafFilterProxyModel::addExtraFilter(FilterFunction func)
{
	mExtraFilters << func;
	invalidateFilter();
}

void LeafFilterProxyModel::clearExemptions()
{
	mExemptions.clear();
	invalidateFilter();
}

bool LeafFilterProxyModel::isExempt(int sourceRow, const QModelIndex& sourceParent) const
{
	// Run through any exempted indexes
	for (int col = 0, len=sourceModel()->columnCount(sourceParent); col < len; col++)
	{
	    const auto sourceIndex = sourceModel()->index(sourceRow, col, sourceParent);

		//const auto sourceIndex = sourceParent.child(sourceRow, col);
		if (mExemptions.contains(sourceIndex))
			return true;
	}
	return false;
}


bool nap::qt::LeafFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	return QSortFilterProxyModel::lessThan(left, right);
}


