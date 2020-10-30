/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "filterpopup.h"
#include "qtutils.h"
#include <QKeyEvent>
#include <QtDebug>

using namespace nap::qt;


FilterPopup::FilterPopup(QWidget* parent) : QMenu(parent)
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

	auto& model = mFilterTree.getFilterModel();
	connect(&model, &QSortFilterProxyModel::rowsRemoved,
			[this](const QModelIndex& parent, int first, int last) { updateSize(); });
	connect(&model, &QSortFilterProxyModel::rowsInserted,
			[this](const QModelIndex& parent, int first, int last) { updateSize(); });
	connect(&mFilterTree, &FilterTreeView::doubleClicked, [this] (auto index) { accept(); });
	updateSize();
}


QString FilterPopup::show(QWidget* parent, const QStringList& items)
{
	return show(parent, items, QCursor::pos());
}


QString FilterPopup::show(QWidget* parent, const QStringList& items, QPoint pos)
{
	FilterPopup popup(parent);
	popup.setItems(items);
	popup.exec(pos);
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


void FilterPopup::setItems(const QStringList& items)
{
	if (mModel)
	{
		mFilterTree.setModel(nullptr);
		delete mModel;
		mModel = nullptr;
	}

	mModel = new QStringListModel(items);
	mFilterTree.setModel(mModel);
}


void FilterPopup::moveSelection(int d)
{
	auto& tree = mFilterTree.getTreeView();
	int row = tree.currentIndex().row();
	int nextRow = qMax(0, qMin(row + d, mFilterTree.getFilterModel().rowCount() - 1));
	if (row == nextRow)
		return;

	tree.setCurrentIndex(mFilterTree.getFilterModel().index(nextRow, 0));
}


void FilterPopup::accept()
{
	auto& mTree = mFilterTree.getTreeView();
	auto idx = mTree.currentIndex();
	mChoice = mTree.model()->data(idx).toString();
	close();
}


void FilterPopup::updateSize()
{
	auto& model = mFilterTree.getFilterModel();

	// Ensure there is always something selected
	auto& tree = *dynamic_cast<FilterTree_*>(&mFilterTree.getTreeView());
	if (!tree.currentIndex().isValid())
		tree.setCurrentIndex(model.index(0, 0));

	// Adjust size based on contents
	int height = mFilterTree.getLineEdit().height() + mBottomMargin;
	int rowCount = model.rowCount();
	if (rowCount > 0)
	{
		QItemSelection selection;
		selection.select(model.index(0, 0), model.index(rowCount - 1, 0));
		auto rect = tree.visualRectFor(selection);
		height += rect.height();
	}

	height = qMin(height, mMaxHeight);
	setFixedSize(300, height);
	adjustSize();
}
