#include "filtertreeview.h"

#include <cassert>

#include <QMenu>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QtDebug>

#include "qtutils.h"

napqt::FilterTreeView::FilterTreeView(QTreeView* treeview)
{
	if (treeview == nullptr)
		treeview = new QTreeView(this);

	mTreeView = treeview;
	mTreeView->setParent(this);

	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	setLayout(&mLayout);

	mSortFilter.setFilterCaseSensitivity(Qt::CaseInsensitive);
	mSortFilter.setFilterKeyColumn(-1); // Filter all columns

	mLineEditFilter.setPlaceholderText("filter");
	mLineEditFilter.setClearButtonEnabled(true);
	connect(&mLineEditFilter, &QLineEdit::textChanged, this, &FilterTreeView::onFilterChanged);
	mLayout.addWidget(&mLineEditFilter);

	mTreeView->setModel(&mSortFilter);

	mLayout.addWidget(mTreeView);

	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, &QWidget::customContextMenuRequested, this, &FilterTreeView::onCustomContextMenuRequested);

	connect(mTreeView, &QTreeView::doubleClicked, this, &FilterTreeView::doubleClicked);
}

void napqt::FilterTreeView::setModel(QAbstractItemModel* model)
{
	mSortFilter.setSourceModel(model);
}

QStandardItemModel* napqt::FilterTreeView::getModel() const
{
	return dynamic_cast<QStandardItemModel*>(mSortFilter.sourceModel());
}

void napqt::FilterTreeView::selectAndReveal(QStandardItem* item)
{
	if (item == nullptr)
		return;

	QModelIndex idx = getFilterModel().mapFromSource(item->index());
	if (!idx.isValid())
	{
		// Probably filtered out, add an exception and try again
		mSortFilter.exemptSourceIndex(item->index());
		idx = getFilterModel().mapFromSource(item->index());
		if (!idx.isValid())
		{
			qInfo() << "Nothing to select";
			return;
		}
	}
	// We are going to select an entire row
	auto botRight = getFilterModel().index(idx.row(), getFilterModel().columnCount(idx.parent()) - 1, idx.parent());
    getTreeView().selectionModel()->select(QItemSelection(idx, botRight), QItemSelectionModel::ClearAndSelect);
    getTreeView().scrollTo(idx);
}


QStandardItem* napqt::FilterTreeView::getSelectedItem()
{
	for (auto idx : getSelectedIndexes())
		return getModel()->itemFromIndex(idx);
	return nullptr;
}


QList<QStandardItem*> napqt::FilterTreeView::getSelectedItems() const
{
	QList<QStandardItem*> ret;
	for (auto idx : getSelectedIndexes())
		ret.append(getModel()->itemFromIndex(idx));
	return ret;
}

QList<QModelIndex> napqt::FilterTreeView::getSelectedIndexes() const
{
	QList<QModelIndex> ret;
	for (auto idx : getSelectionModel()->selectedRows())
		ret.append(mSortFilter.mapToSource(idx));
	return ret;
}

void napqt::FilterTreeView::onFilterChanged(const QString& text)
{
	mSortFilter.setFilterRegExp(text);
	mSortFilter.clearExemptions();
	mTreeView->expandAll();
	setTopItemSelected();
}

void napqt::FilterTreeView::onExpandSelected()
{
	for (auto& idx : getSelectedIndexes())
		expandChildren(mTreeView, idx, true);
}

void napqt::FilterTreeView::onCollapseSelected()
{
	for (auto& idx : getSelectedIndexes())
		expandChildren(mTreeView, idx, false);
}


void napqt::FilterTreeView::onCustomContextMenuRequested(const QPoint& pos)
{
	QMenu menu;

	if (mMenuHookFn != nullptr)
		mMenuHookFn(menu);

	auto expandAllAction = menu.addAction("Expand All");
	connect(expandAllAction, &QAction::triggered, this, &FilterTreeView::onExpandSelected);

	auto collapseAllAction = menu.addAction("Collapse");
	connect(collapseAllAction, &QAction::triggered, this, &FilterTreeView::onCollapseSelected);

	menu.exec(mapToGlobal(pos));
}

void napqt::FilterTreeView::setIsItemSelector(bool b)
{
	if (b) {
		setTopItemSelected();
	}
}

void napqt::FilterTreeView::setTopItemSelected()
{
	auto model = mTreeView->model();
	if (model->rowCount() == 0)
		return;

	auto leftIndex = model->index(0, 0);
	auto rightIndex = model->index(0, model->columnCount() - 1);
	QItemSelection selection(leftIndex, rightIndex);
	getSelectionModel()->select(selection, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

QWidget& napqt::FilterTreeView::getCornerWidget()
{
	return mCornerWidget;
}

