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
	mTreeView->setParent(this);
	mTreeView->setSortingEnabled(false);

	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	setLayout(&mLayout);

	mProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
	mProxyModel.setFilterKeyColumn(-1); // Filter all columns

	mLineEditFilter.setPlaceholderText("filter...");
	mLineEditFilter.setClearButtonEnabled(true);
	connect(&mLineEditFilter, &QLineEdit::textChanged, this, &FilterTreeView::onFilterChanged);
	mLayout.addWidget(&mLineEditFilter);
	mTreeView->setModel(&mProxyModel);
	mLayout.addWidget(mTreeView);
	setContextMenuPolicy(Qt::CustomContextMenu);

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
	for (auto idx : getSelectedIndexes())
		return getModel()->itemFromIndex(idx);
	return nullptr;
}


QList<QStandardItem*> FilterTreeView::getSelectedItems() const
{
	QList<QStandardItem*> ret;
	for (auto idx : getSelectedIndexes())
		ret.append(getModel()->itemFromIndex(idx));
	return ret;
}


QList<QModelIndex> FilterTreeView::getSelectedIndexes() const
{
	QList<QModelIndex> ret;
	for (auto idx : getSelectionModel()->selectedRows())
		ret.append(mProxyModel.mapToSource(idx));
	return ret;
}


void FilterTreeView::onFilterChanged(const QString& text)
{
	mProxyModel.setFilterRegularExpression(text);
	mProxyModel.clearExemptions();
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

	auto last_rect = mTreeView->visualRect(last_idx);
	auto firs_rect = mTreeView->visualRect(mTreeView->model()->index(0, 0));

	return { firs_rect.topLeft(), last_rect.bottomRight() };
}

