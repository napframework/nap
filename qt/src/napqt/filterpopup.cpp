/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "filterpopup.h"
#include "qtutils.h"
#include <QKeyEvent>
#include <QtDebug>
#include <QScreen>

using namespace nap::qt;


FilterPopup::FilterPopup(StringModel::Entries&& entries, QWidget* parent) : QMenu(parent)
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	mLayout.setAlignment(Qt::AlignTop);

	mModel = std::make_unique<StringModel>(std::move(entries));
	mFilterTree.setModel(mModel.get());
	mFilterTree.getTreeView().setHeaderHidden(true);
	mFilterTree.getTreeView().setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	mLayout.addWidget(&mFilterTree);

	connect(&mFilterTree.getProxyModel(), &QSortFilterProxyModel::rowsRemoved, [this](const QModelIndex& parent, int first, int last) { computeSize(); });
	connect(&mFilterTree.getProxyModel(), &QSortFilterProxyModel::rowsInserted, [this](const QModelIndex& parent, int first, int last) { computeSize(); });
	connect(&mFilterTree.getTreeView(), &QTreeView::expanded, [this](const QModelIndex& index) {computeSize(); });
	connect(&mFilterTree.getTreeView(), &QTreeView::collapsed, [this](const QModelIndex& index) {computeSize(); });
	connect(&mFilterTree, &FilterTreeView::doubleClicked, [this](auto index) { accept(); });

	mFilterTree.getTreeView().expandAll();
}


QString FilterPopup::show(QWidget* parent, StringModel::Entries&& items)
{
	return show(parent, std::move(items), QCursor::pos());
}


QString FilterPopup::show(QWidget* parent, StringModel::Entries&& entries, QPoint pos)
{
	// Create popup
	FilterPopup popup(std::move(entries), parent);

	// Ensure popup is within display bounds
	QRect geo = QGuiApplication::screenAt(pos)->geometry();
	QSize size = popup.size();

	// Within display bounds over x
	QPoint f_pos = pos;
	if ((pos.x() + size.width()) > geo.right())
	{
		int new_x = pos.x() - (pos.x() - geo.right()) - size.width();
		f_pos.setX(new_x);
	}

	// Within display bounds over y
	if ((pos.y() + size.height()) > geo.bottom())
	{
		int new_y = pos.y() - (pos.y() - geo.bottom()) - size.height();
		f_pos.setY(new_y);
	}

	// Show
	popup.exec(f_pos);
	return popup.mChoice;
}


void FilterPopup::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		{
			event->accept();
			accept();
			return;
		}
	}
	QMenu::keyPressEvent(event);
}


void FilterPopup::showEvent(QShowEvent* event)
{
	QMenu::showEvent(event);
	mFilterTree.getLineEdit().setFocus();
}


void FilterPopup::accept()
{
	auto& mTree = mFilterTree.getTreeView();
	auto idx = mTree.currentIndex();
	mChoice = mTree.model()->data(idx).toString();
	close();
}


// Returns the last visible item in the tree view or invalid model index if not found any.
static QModelIndex lastVisibleItem(QTreeView* view, const QModelIndex& index = QModelIndex())
{
	QAbstractItemModel* model = view->model();
	int rowCount = model->rowCount(index);
	if (rowCount > 0) {
		// Find the last item in this level of hierarchy.
		QModelIndex lastIndex = model->index(rowCount - 1, 0, index);
		if (model->hasChildren(lastIndex) && view->isExpanded(lastIndex)) {
			// There is even deeper hierarchy. Drill down with recursion.
			return lastVisibleItem(view, lastIndex);
		}
		else {
			// Test the last item in the tree.
			return lastIndex;
		}
	}
	else {
		return QModelIndex();
	}
}


void FilterPopup::computeSize()
{
	// Update layout
	mLayout.update();

	// Ensure there is always something selected
	auto& model = mFilterTree.getProxyModel();
	auto& tree = mFilterTree.getTreeView();
	if (!tree.currentIndex().isValid())
		tree.setCurrentIndex(model.index(0, 0));

	// Adjust size based on contents
	int height = mFilterTree.getLineEdit().sizeHint().height();
	auto vis_rect = mFilterTree.getVisibleRect();

	height += vis_rect.isValid() ? vis_rect.height() : height * 2;
	setFixedSize(300, qMin(height, 500));
	adjustSize();
}


QVariant StringModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
	{
		return static_cast<StringModel::Item*>(itemFromIndex(index))->mEntry.mText;
	}
	case Qt::FontRole:
	{
		auto* item = static_cast<StringModel::Item*>(itemFromIndex(index));
		if (item->hasChildren())
		{
			QFont font(item->font());
			font.setBold(true);
			return font;
		}
		return QStandardItemModel::data(index, role);
	}
	case Qt::DecorationRole:
	{
		auto* item = static_cast<StringModel::Item*>(itemFromIndex(index));
		return item->mEntry.mIcon.isNull() ?
			QStandardItemModel::data(index, role) :
			item->mEntry.mIcon;
	}
	case Qt::ToolTipRole:
	{
		auto* item = static_cast<StringModel::Item*>(itemFromIndex(index));
		return item->mEntry.mTooltip.isNull() ? QStandardItemModel::data(index, role) :
			item->mEntry.mTooltip;
	}
	default:
		return QStandardItemModel::data(index, role);
	}
}


nap::qt::StringModel::StringModel(Entries&& entries)
{
	for (auto& entry : entries)
	{
		appendRow(new Item(std::move(entry)));
	}
}


void nap::qt::StringModel::sort(Entries& ioEntries, bool reverseSort)
{
	// Sort list
	std::sort(ioEntries.begin(), ioEntries.end(), [reverseSort](const auto& a, const auto& b)
		{
			return reverseSort ? b.mText < a.mText : a.mText < b.mText;
		});

	// Recursively sort children
	for (auto& entry : ioEntries)
		StringModel::sort(entry.mChildren);
}


nap::qt::StringModel::Item::Item(Entry&& entry) : QStandardItem(entry.mText), mEntry(std::move(entry))
{
	// Add children
	for (auto& child_entry : mEntry.mChildren)
		appendRow(new StringModel::Item(std::move(child_entry)));

	// Clear invalidated children
	mEntry.mChildren.clear();
}
