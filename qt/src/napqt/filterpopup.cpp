/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "filterpopup.h"
#include "qtutils.h"
#include <QKeyEvent>
#include <QtDebug>
#include <QScreen>

using namespace nap::qt;


FilterPopup::FilterPopup(StringModel::Entries&& entries, QWidget* parent) : QDialog(parent)
{
	// Create model
	mModel = std::make_unique<StringModel>(std::move(entries));
	mFilterTree.setModel(mModel.get());
	mFilterTree.getTreeView().setHeaderHidden(true);
	mFilterTree.getTreeView().setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	mFilterTree.getTreeView().setRootIsDecorated(mModel->nested());

	// Install layout
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	mLayout.setAlignment(Qt::AlignTop);
	mLayout.addWidget(&mFilterTree);
	mLayout.activate();

	// Ensure widget size is re-computed when sorting ends
	connect(&mFilterTree, &FilterTreeView::doubleClicked, [this](auto index) { accept(); });
	connect(&mFilterTree, &FilterTreeView::sortingEnded, [this]() { mFilterTree.getTreeView().expandAll(); });

	// Handle key up and down for item selection
	mFilterTree.getLineEdit().installEventFilter(this);

	computeSize();
}


QString FilterPopup::show(QWidget* parent, StringModel::Entries&& items)
{
	return show(parent, std::move(items), QCursor::pos());
}


QString FilterPopup::show(QWidget* parent, StringModel::Entries&& entries, QPoint pos)
{
	// Create popup
	FilterPopup popup(std::move(entries), parent);
	popup.setWindowFlags(Qt::Popup);

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
	popup.move(f_pos);
	popup.exec();
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
	QDialog::keyPressEvent(event);
}


void FilterPopup::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
	mFilterTree.getLineEdit().setFocus();
}


void FilterPopup::accept()
{
	auto& mTree = mFilterTree.getTreeView();
	auto idx = mTree.currentIndex();
	mChoice = mTree.model()->data(idx).toString();
	close();
}


void FilterPopup::computeSize()
{
	// Update layout
	mFilterTree.getTreeView().expandAll();

	// Ensure there is always something selected
	auto& model = mFilterTree.getProxyModel();
	auto& tree = mFilterTree.getTreeView();
	if (!tree.currentIndex().isValid())
		tree.setCurrentIndex(model.index(0, 0));

	// Adjust size based on contents
	auto vis_rect = mFilterTree.getVisibleRect();
	int width = mModel->nested() ? 350 : 300;
	int height = mFilterTree.getLineEdit().sizeHint().height();
	height += vis_rect.isValid() ? vis_rect.height() : height * 2;
	height = qMin(height, 500);
	setMinimumSize(width, height);
	resize(width, height);
}


bool FilterPopup::eventFilter(QObject* watched, QEvent* event)
{
	// Only handle key-presses
	if (event->type() == QEvent::KeyPress)
	{
		// Cast and handle
		assert(qobject_cast<QLineEdit*>(watched) != nullptr);
		switch (static_cast<QKeyEvent*>(event)->key())
		{
		case Qt::Key_Down:
		case Qt::Key_Up:
			focusNextChild();
			return true;
		}
	}
	return false;
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
		if (!entry.mChildren.empty())
			mNested = true;
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
	setEditable(false);
	for (auto& child_entry : mEntry.mChildren)
		appendRow(new StringModel::Item(std::move(child_entry)));

	// Clear invalidated children
	mEntry.mChildren.clear();
}
