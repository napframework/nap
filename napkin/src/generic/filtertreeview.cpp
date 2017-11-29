#include "filtertreeview.h"
#include <QMenu>
#include <QTimer>
#include <assert.h>


napkin::FilterTreeView::FilterTreeView()
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	setLayout(&mLayout);

	mSortFilter.setFilterCaseSensitivity(Qt::CaseInsensitive);
	mSortFilter.setFilterKeyColumn(-1); // Filter all columns

	mLineEditFilter.setPlaceholderText("filter");
	mLineEditFilter.setClearButtonEnabled(true);
	connect(&mLineEditFilter, &QLineEdit::textChanged, this, &FilterTreeView::onFilterChanged);
	mLayout.addWidget(&mLineEditFilter);

	mTreeView.setModel(&mSortFilter);

	mLayout.addWidget(&mTreeView);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &FilterTreeView::onCustomContextMenuRequested);
}

void napkin::FilterTreeView::setModel(QStandardItemModel* model) { mSortFilter.setSourceModel(model); }

QStandardItemModel* napkin::FilterTreeView::getModel() const
{
	return dynamic_cast<QStandardItemModel*>(mSortFilter.sourceModel());
}

void napkin::FilterTreeView::selectAndReveal(QStandardItem* item)
{
	if (item == nullptr)
		return;
	auto idx = getFilterModel().mapFromSource(item->index());
	// We are going to select an entire row
	auto botRight = getFilterModel().index(idx.row(), getFilterModel().columnCount(idx.parent()) - 1, idx.parent());
    getTreeView().selectionModel()->select(QItemSelection(idx, botRight), QItemSelectionModel::ClearAndSelect);
    getTreeView().scrollTo(idx);
}


QStandardItem* napkin::FilterTreeView::getSelectedItem()
{
	for (auto idx : getSelectedIndexes())
		return getModel()->itemFromIndex(idx);
	return nullptr;
}


QList<QStandardItem*> napkin::FilterTreeView::getSelectedItems() const
{
	QList<QStandardItem*> ret;
	for (auto idx : getSelectedIndexes())
		ret.append(getModel()->itemFromIndex(idx));
	return ret;
}

QList<QModelIndex> napkin::FilterTreeView::getSelectedIndexes() const
{
	QList<QModelIndex> ret;
	for (auto idx : getSelectionModel()->selectedRows())
		ret.append(mSortFilter.mapToSource(idx));
	return ret;
}

void napkin::FilterTreeView::onFilterChanged(const QString& text)
{
	mSortFilter.setFilterRegExp(text);
	mTreeView.expandAll();
}


void napkin::FilterTreeView::onExpandSelected()
{
	for (auto& idx : getSelectedIndexes())
		expandChildren(&mTreeView, idx, true);
}

void napkin::FilterTreeView::onCollapseSelected()
{
	for (auto& idx : getSelectedIndexes())
		expandChildren(&mTreeView, idx, false);
}

void napkin::FilterTreeView::expandChildren(QTreeView* view, const QModelIndex& index, bool expanded)
{
	if (!index.isValid())
		return;

	for (int i = 0, len = index.model()->rowCount(index); i < len; i++)
		expandChildren(view, index.child(i, 0), expanded);

	if (expanded && !view->isExpanded(index))
		view->expand(index);
	else if (view->isExpanded(index))
		view->collapse(index);
}

void napkin::FilterTreeView::onCustomContextMenuRequested(const QPoint& pos)
{
	QMenu menu;

	if (mMenuHookFn != nullptr)
		mMenuHookFn(menu);

	mActionExpandAll.setText("Expand All");
	menu.addAction(&mActionExpandAll);
	mActionCollapseAll.setText("Collapse");
	menu.addAction(&mActionCollapseAll);

	menu.exec(mapToGlobal(pos));
}
