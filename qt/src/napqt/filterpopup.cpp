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

	auto& tree = mFilterTree.getTreeView();
	tree.setRootIsDecorated(false);
	tree.setHeaderHidden(true);
	tree.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	mLayout.addWidget(&mFilterTree);
	mLayout.activate();

	auto& model = mFilterTree.getProxyModel();
	connect(&model, &QSortFilterProxyModel::rowsRemoved, [this](const QModelIndex& parent, int first, int last) { computeSize(); });
	connect(&model, &QSortFilterProxyModel::rowsInserted, [this](const QModelIndex& parent, int first, int last) { computeSize(); });
	connect(&mFilterTree, &FilterTreeView::doubleClicked, [this] (auto index) { accept(); });

	mModel = std::make_unique<StringModel>(std::move(entries));
	mFilterTree.setModel(mModel.get());
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
		case Qt::Key_Down:
		{
			moveSelection(1);
			event->accept();
			return;
		}
		case Qt::Key_Up:
		{
			moveSelection(-1);
			event->accept();
			return;
		}
		case Qt::Key_Enter:
		case Qt::Key_Return:
		{
			event->accept();
			accept();
		}
		default:
			QMenu::keyPressEvent(event);
	}
}


void FilterPopup::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	mFilterTree.getLineEdit().setFocus();
}


void FilterPopup::moveSelection(int d)
{
	auto& tree = mFilterTree.getTreeView();
	int row = tree.currentIndex().row();
	int nextRow = qMax(0, qMin(row + d, mFilterTree.getProxyModel().rowCount() - 1));
	if (row == nextRow)
		return;

	tree.setCurrentIndex(mFilterTree.getProxyModel().index(nextRow, 0));
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
	mLayout.update();

	// Ensure there is always something selected
	auto& model = mFilterTree.getProxyModel();
	auto& tree = *qobject_cast<FilterTree_*>(&mFilterTree.getTreeView());
	if (!tree.currentIndex().isValid())
		tree.setCurrentIndex(model.index(0, 0));

	// Adjust size based on contents
	int height = mFilterTree.getLineEdit().sizeHint().height();
	int rowCount = model.rowCount();
	if (rowCount > 0)
	{
		QItemSelection selection;
		selection.select(model.index(0, 0), model.index(rowCount-1, 0));
		auto rect = tree.visualRectFor(selection);
		height += rect.height();
	}
	else
	{
		height *= 2;
	}

	setFixedSize(300, qMin(height, 500));
	adjustSize();
}


QVariant StringModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		{
			auto* string_item = static_cast<StringModel::Item*>(itemFromIndex(index));
			assert(string_item != nullptr);
			return string_item->mEntry.mText;
		}
	case Qt::ToolTipRole:
		{
			auto* string_item = static_cast<StringModel::Item*>(itemFromIndex(index));
			assert(string_item != nullptr);
			return string_item->mEntry.mTooltip.isNull() ? QStandardItemModel::data(index, role) :
				string_item->mEntry.mTooltip;
		}
	default:
		return QStandardItemModel::data(index, role);
	}
}


nap::qt::StringModel::StringModel(Entries&& items)
{
	for (auto& item : items)
	{
		appendRow(new Item(std::move(item)));
	}
}


void nap::qt::StringModel::sort(Entries& ioEntries, bool reverseSort)
{
	std::sort(ioEntries.begin(), ioEntries.end(), [reverseSort](const auto& a, const auto& b)
		{
			return reverseSort ? b.mText < a.mText : a.mText < b.mText;
		});
}
