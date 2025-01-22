/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "filtertreeview.h"
#include "qtutils.h"
#include <cassert>
#include <QMenu>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QtDebug>

using namespace nap::qt;

FilterTreeView::FilterTreeView(QTreeView* treeview)
{
	mTreeView = new QTreeView(this);
	mTreeView->setSortingEnabled(false);
	mTreeView->setFocusPolicy(Qt::StrongFocus);

	mProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
	mProxyModel.setFilterKeyColumn(-1); // Filter all columns
	mTreeView->setModel(&mProxyModel);

	mLineEditFilter.setPlaceholderText("filter...");
	mLineEditFilter.setClearButtonEnabled(true);
	mLineEditFilter.setFocusPolicy(Qt::StrongFocus);

	mLayout.addWidget(&mLineEditFilter);
	mLayout.addWidget(mTreeView);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	setLayout(&mLayout);

	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(&mLineEditFilter, &QLineEdit::textChanged, this, &FilterTreeView::onFilterChanged);
	connect(this, &QWidget::customContextMenuRequested, this, &FilterTreeView::onCustomContextMenuRequested);
	connect(mTreeView, &QTreeView::doubleClicked, this, &FilterTreeView::doubleClicked);
}

void FilterTreeView::setModel(QAbstractItemModel* model)
{
	mProxyModel.setSourceModel(model);
}

QStandardItemModel* FilterTreeView::getModel() const
{
	return qobject_cast<QStandardItemModel*>(mProxyModel.sourceModel());
}


void FilterTreeView::select(const QStandardItem* item, bool expand)
{
	if (item == nullptr)
		return;

	QModelIndex idx = getProxyModel().mapFromSource(item->index());
	if (!idx.isValid())
	{
		// Probably filtered out, add an exception and try again
		mProxyModel.exemptSourceIndex(item->index());
		idx = getProxyModel().mapFromSource(item->index());
		if (!idx.isValid())
			return;
	}

	// We are going to select an entire row
	auto botRight = getProxyModel().index(idx.row(), getProxyModel().columnCount(idx.parent()) - 1, idx.parent());
    getTreeView().selectionModel()->select(QItemSelection(idx, botRight), QItemSelectionModel::ClearAndSelect);
	if (expand) { getTreeView().expand(idx); }
    getTreeView().scrollTo(idx);
}


QStandardItem* FilterTreeView::getSelectedItem()
{
	for (const auto& idx : getSelectedIndexes())
		return getModel()->itemFromIndex(idx);
	return nullptr;
}


QList<QStandardItem*> FilterTreeView::getSelectedItems() const
{
	QList<QStandardItem*> ret;
	for (const auto& idx : getSelectedIndexes())
		ret.append(getModel()->itemFromIndex(idx));
	return ret;
}


QList<QModelIndex> FilterTreeView::getSelectedIndexes() const
{
	QList<QModelIndex> ret;
	for (const auto& idx : getSelectionModel()->selectedRows())
		ret.append(mProxyModel.mapToSource(idx));
	return ret;
}


void FilterTreeView::onFilterChanged(const QString& text)
{
	sortingStarted();
	mProxyModel.setFilterRegularExpression(text);
	mProxyModel.clearExemptions();
	sortingEnded();
}


void FilterTreeView::onCustomContextMenuRequested(const QPoint& pos)
{
	QMenu menu;
	if (mMenuHookFn != nullptr)
		mMenuHookFn(menu);
	menu.exec(mapToGlobal(pos));
}


QWidget& FilterTreeView::getCornerWidget()
{
	return mCornerWidget;
}


void nap::qt::FilterTreeView::enableSorting(LeafFilterProxyModel::SortingFunction sorter /*= nullptr*/)
{
	mProxyModel.setSorter(sorter);
	mTreeView->setSortingEnabled(true);
}


void nap::qt::FilterTreeView::disableSorting()
{
	mTreeView->setSortingEnabled(false);
}


void nap::qt::FilterTreeView::expand(const QStandardItem& item) const
{
	mTreeView->expand(getProxyModel().mapFromSource(item.index()));
}


QModelIndex nap::qt::FilterTreeView::getLastVisibleItemIndex(const QModelIndex& index) const
{
	int row_count = mProxyModel.rowCount(index);
	if (row_count > 0)
	{
		// Find the last item in this level of hierarchy.
		int col_count = mProxyModel.columnCount(index) - 1;
		QModelIndex last_idx = mProxyModel.index(row_count - 1, col_count, index);
		return mProxyModel.hasChildren(last_idx) && mTreeView->isExpanded(last_idx) ?
			getLastVisibleItemIndex(last_idx) : last_idx;
	}
	return QModelIndex();
}


QRect nap::qt::FilterTreeView::getVisibleRect() const
{
	auto last_idx = getLastVisibleItemIndex();
	if (!last_idx.isValid())
		return QRect();

	// Construct and return visible rect based on top left and bottom right index
	return {
		mTreeView->visualRect(mTreeView->model()->index(0, 0)).topLeft(),
		mTreeView->visualRect(last_idx).bottomRight()
	};
}

