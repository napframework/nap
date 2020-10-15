/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "curvewidget.h"
#include <QHeaderView>
#include <QStylePainter>
#include <QtDebug>


using namespace nap::qt;

void CurveTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QItemDelegate::paint(painter, option, index);
	int iconSize = option.rect.height() - mIconMargins.top() - mIconMargins.bottom();
	int iconDelegateCount = mIconDelegates.size();
	int iconsWidth = iconSize * iconDelegateCount + mIconSpacing * (iconDelegateCount-1);
	int iconsWidthMargins = iconsWidth + mIconMargins.left() + mIconMargins.right();

	QStyleOptionViewItem op(option);
	op.rect = op.rect.adjusted(0, 0, -iconsWidthMargins, 0);

	QItemDelegate::paint(painter, op, index);

	int x = op.rect.right() + mIconMargins.left();
	int y = op.rect.top() + mIconMargins.top();
	for (const auto& iconDelegate : mIconDelegates)
	{
		auto delegate = iconDelegate.get();
		QStyleOptionViewItem iconOp(option);
		iconOp.rect = {x, y, iconSize, iconSize};
		delegate->paint(painter, iconOp, index);
		x += iconSize + mIconSpacing;
	}

}

QSize CurveTreeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QItemDelegate::sizeHint(option, index);
}

void CurveTreeDelegate::addDelegate(std::shared_ptr<CurveTreeIconDelegate> delegate)
{
	mIconDelegates.append(delegate);
}


CurveWidget::CurveWidget(QWidget* parent) : QWidget(parent)
{
	setLayout(&mLayout);
	mLayout.addWidget(&mSplitter);
	mSplitter.addWidget(&mTreeView);
	mSplitter.addWidget(&mCurveView);
	mTreeView.setModel(&mTreeModel);

	connect(&mTreeView, &FilterTreeView::doubleClicked, this, &CurveWidget::onTreeDoubleClicked);

	{
		auto& tree = mTreeView.getTreeView();

		tree.setItemDelegate(&mDelegate);

		auto pal = tree.palette();
		pal.setColor(QPalette::Base, pal.color(QPalette::Window));
		tree.setPalette(pal);

		tree.setHeaderHidden(true);

		tree.setStyleSheet("border: none;");
		AutoSettings::get().exclude(tree.header());

		mDelegate.addDelegate(std::make_shared<CurveTreeIconDelegate>());
		mDelegate.addDelegate(std::make_shared<CurveTreeIconDelegate>());
		mDelegate.addDelegate(std::make_shared<CurveTreeIconDelegate>());
	}
}

void CurveWidget::setModel(AbstractCurveModel* model)
{
	mCurveView.setModel(model);
	mTreeModel.setCurveModel(model);
//	auto header = mTreeView.getTreeView().header();
//	header->setStretchLastSection(false);
//	header->setSectionResizeMode(QHeaderView::Stretch);
//	header->setSectionResizeMode(0, QHeaderView::Stretch);
//	for (int i = 1; i < mTreeModel.columnCount(); i++)
//	{
//		header->setSectionResizeMode(i, QHeaderView::Fixed);
//		header->resizeSection(i, 20);
//	}
}

void CurveWidget::onTreeDoubleClicked(const QModelIndex& idx)
{
	const auto& sourceIndex = mTreeView.getFilterModel().mapToSource(idx);
	auto curve = mTreeModel.curveFromIndex(sourceIndex);
	mCurveView.selectCurves({curve});
}
