#include "curvepanel.h"
#include <memory>
#include <fcurvemodel.h>
#include <appcontext.h>

using namespace napkin;
using namespace napqt;

CurvePanel::CurvePanel(QWidget* parent) : QWidget(parent)
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	setLayout(&mLayout);
	mLayout.addWidget(&mCurveView);

	connect(&AppContext::get(), &AppContext::propertyValueChanged, [this](const PropertyPath path)
	{
		if (&path.getObject() != &mCurveModel->curve().sourceCurve())
			return;
		onCurveUpdated();
	});
}

void CurvePanel::onCurveUpdated()
{
	QList<int> indices;
	for (int i=0, len = mCurveModel->curve().pointCount(); i<len; i++)
		indices << i;
	mListenForCurveChanges = false;
	mCurveModel->curve().pointsChanged(indices, true);
	mListenForCurveChanges = true;
}

void CurvePanel::editCurve(nap::math::FloatFCurve* curve)
{
	if (curve == nullptr)
	{
		mCurveView.setModel(nullptr);
		mCurveModel.reset();
		return;
	}

	mCurveModel = std::make_shared<FloatFCurveModel>(*curve);

	connect(&mCurveModel->curve(), &napkin::FCurve::pointsChanged, [this](QList<int> pointIndexes, bool finished)
	{
		if (!mListenForCurveChanges)
			return;
		// push changes to application
		if (!finished)
			return;
		for (int idx : pointIndexes)
			AppContext::get().propertyValueChanged(mCurveModel->curve().pointPath(idx));
	});

	mCurveView.setModel(mCurveModel.get());
}
