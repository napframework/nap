/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "napkinfiltertree.h"
#include "standarditemsproperty.h"
#include "commands.h"
#include "appcontext.h"
#include "propertypath.h"

#include <QMimeData>
#include <panels/inspectorpanel.h>


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
