#include "filtertreeview.h"
#include "qtutils.h"
#include <QMenu>
#include <QTimer>
#include <assert.h>
#include <nap/logger.h>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <appcontext.h>
#include <standarditemsproperty.h>
#include <panels/inspectorpanel.h>
#include <commands.h>

napkin::_FilterTreeView::_FilterTreeView() : QTreeView()
{
	setAcceptDrops(true);
}

void napkin::_FilterTreeView::dragEnterEvent(QDragEnterEvent* event)
{
	QAbstractItemView::dragEnterEvent(event);
}

void napkin::_FilterTreeView::dragMoveEvent(QDragMoveEvent* event)
{
	// Re-enable if valid drop
	setDropIndicatorShown(false);

	auto idx = indexAt(event->pos());
	if (idx.isValid())
	{
		auto filter_model = dynamic_cast<QSortFilterProxyModel*>(model());
		assert(filter_model != nullptr);
		auto item_model = dynamic_cast<QStandardItemModel*>(filter_model->sourceModel());
		assert(item_model != nullptr);
		auto item = item_model->itemFromIndex(filter_model->mapToSource(idx));

		auto drag_index = currentIndex();
		auto drag_item = item_model->itemFromIndex(filter_model->mapToSource(drag_index));

		auto pathitem = dynamic_cast<PropertyPathItem*>(item);
		if (pathitem != nullptr && drag_item->parent() == pathitem->parent())
		{
			setDropIndicatorShown(true);
			event->acceptProposedAction();
		}
	}

	QTreeView::dragMoveEvent(event);
}

void napkin::_FilterTreeView::dropEvent(QDropEvent* event)
{

	auto drop_index = indexAt(event->pos());
	if (!drop_index.isValid())
		return;


	// User may have dropped on column > 0, convert to column 0
	drop_index = drop_index.model()->index(drop_index.row(), 0, drop_index.parent());

	auto filter_model = dynamic_cast<QSortFilterProxyModel*>(model());
	assert(filter_model != nullptr);
	auto item_model = dynamic_cast<QStandardItemModel*>(filter_model->sourceModel());
	assert(item_model != nullptr);

	auto drop_item = item_model->itemFromIndex(filter_model->mapToSource(drop_index));

	// The current index is the one where we started dragging
	auto curr_index = currentIndex();
	// We might have selected another column, get the leftmost index instead
	auto drag_index = curr_index.model()->index(curr_index.row(), 0, curr_index.parent());
	assert(drop_index.model() == drag_index.model());
	auto drag_item = item_model->itemFromIndex(filter_model->mapToSource(drag_index));

	auto pathitem = dynamic_cast<PropertyPathItem*>(drop_item);
	if (pathitem == nullptr)
		return;

	auto arrayitem = dynamic_cast<ArrayPropertyItem*>(pathitem->parent());
	auto array_path = arrayitem->getPath();

	if (drag_item->parent() != pathitem->parent())
		return;
	auto dropped_path = pathitem->data(Qt::UserRole);

	// Dropped above or below item?
	DropIndicatorPosition dropIndicator = dropIndicatorPosition();
	int target_row = drop_index.row();
	switch (dropIndicator)
	{
		case QAbstractItemView::AboveItem:
			break;
		case QAbstractItemView::BelowItem:
			target_row += 1;
			break;
		case QAbstractItemView::OnItem:
		case QAbstractItemView::OnViewport:
			break;
	}


	auto dragged_path = QString::fromLocal8Bit(event->mimeData()->data(sNapkinMimeData));
	AppContext::get().executeCommand(new ArrayMoveElementCommand(array_path, drag_index.row(), target_row));
}


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

void napkin::FilterTreeView::setModel(QStandardItemModel* model)
{
	mSortFilter.setSourceModel(model);
}

QStandardItemModel* napkin::FilterTreeView::getModel() const
{
	return dynamic_cast<QStandardItemModel*>(mSortFilter.sourceModel());
}

void napkin::FilterTreeView::selectAndReveal(QStandardItem* item)
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
			nap::Logger::warn("Nothing to select...");
			return;
		}
	}
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
	mSortFilter.clearExemptions();
	mTreeView.expandAll();
	setTopItemSelected();
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


void napkin::FilterTreeView::onCustomContextMenuRequested(const QPoint& pos)
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

void napkin::FilterTreeView::setIsItemSelector(bool b)
{
	mIsItemSelector = b;
	if (b) {
		setTopItemSelected();
	}
}

void napkin::FilterTreeView::setTopItemSelected()
{
	auto model = mTreeView.model();
	if (model->rowCount() == 0)
		return;

	auto leftIndex = model->index(0, 0);
	auto rightIndex = model->index(0, model->columnCount() - 1);
	QItemSelection selection(leftIndex, rightIndex);
	getSelectionModel()->select(selection, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

QWidget& napkin::FilterTreeView::getCornerWidget()
{
	return mCornerWidget;
}

