#include "curvepanel.h"

#include <QtDebug>

using namespace napqt;

CurvePanel::CurvePanel(QWidget* parent) : QWidget(parent)
{
	setLayout(&mLayout);
	mLayout.addWidget(&mSplitter);
	mSplitter.addWidget(&mTreeView);
	mSplitter.addWidget(&mCurveView);
	mTreeView.setModel(&mTreeModel);

	connect(&mTreeView, &FilterTreeView::doubleClicked, this, &CurvePanel::onTreeDoubleClicked);
}

void CurvePanel::setModel(AbstractCurveModel* model)
{
	mCurveView.setModel(model);
	mTreeModel.setCurveModel(model);
}

void CurvePanel::onTreeDoubleClicked(const QModelIndex& idx)
{
	const auto& sourceIndex = mTreeView.getFilterModel().mapToSource(idx);
	auto curve = mTreeModel.curveFromIndex(sourceIndex);
	mCurveView.selectCurves({curve});
}
