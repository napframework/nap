#include "curvepanel.h"

#include <QHeaderView>
#include <QStylePainter>
#include <QtDebug>


using namespace napqt;

void CurveTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.column() == 0)
	{
		QItemDelegate::paint(painter, option, index);
		return;
	}
	if (index.column() > 0)
	{
		int mgn = 3;

		auto col = index.data(CurveTreeRole::ColorRole).value<QColor>();
		if (option.state & QStyle::State_Selected)
		{
			painter->fillRect(option.rect, option.palette.highlight());
		}

		painter->fillRect(option.rect.adjusted(mgn, mgn, -mgn, -mgn), QBrush(col));
		painter->setPen(QPen(Qt::gray, 1));
		painter->drawRect(option.rect.adjusted(mgn, mgn, -mgn, -mgn));
	}

}

QSize CurveTreeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QItemDelegate::sizeHint(option, index);
}


CurvePanel::CurvePanel(QWidget* parent) : QWidget(parent)
{
	setLayout(&mLayout);
	mLayout.addWidget(&mSplitter);
	mSplitter.addWidget(&mTreeView);
	mSplitter.addWidget(&mCurveView);
	mTreeView.setModel(&mTreeModel);

	connect(&mTreeView, &FilterTreeView::doubleClicked, this, &CurvePanel::onTreeDoubleClicked);

	{
		auto& tree = mTreeView.getTreeView();

		tree.setItemDelegate(&mDelegate);

		auto pal = tree.palette();
		pal.setColor(QPalette::Base, pal.color(QPalette::Window));
		tree.setPalette(pal);

		tree.setHeaderHidden(true);

		tree.setStyleSheet("border: none;");
		AutoSettings::get().exclude(tree.header());
	}
}

void CurvePanel::setModel(AbstractCurveModel* model)
{
	mCurveView.setModel(model);
	mTreeModel.setCurveModel(model);
	auto header = mTreeView.getTreeView().header();
	header->setStretchLastSection(false);
	header->setSectionResizeMode(QHeaderView::Stretch);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	for (int i = 1; i < mTreeModel.columnCount(); i++)
	{
		header->setSectionResizeMode(i, QHeaderView::Fixed);
		header->resizeSection(i, 20);
	}
}

void CurvePanel::onTreeDoubleClicked(const QModelIndex& idx)
{
	const auto& sourceIndex = mTreeView.getFilterModel().mapToSource(idx);
	auto curve = mTreeModel.curveFromIndex(sourceIndex);
	mCurveView.selectCurves({curve});
}
