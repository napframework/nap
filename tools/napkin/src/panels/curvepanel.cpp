/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "curvepanel.h"

#include <memory>
#include <fcurvemodel.h>
#include <appcontext.h>
#include <napqt/qtutils.h>

using namespace napkin;
using namespace nap::qt;

CurvePanel::CurvePanel(QWidget* parent) : QWidget(parent)
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	setLayout(&mLayout);
	mLayout.addWidget(&mCurveView);

	connect(&AppContext::get(), &AppContext::propertyValueChanged, [this](const PropertyPath path)
	{
		if (!mCurveModel)
			return;

		if (!mListenForPropertyChanges)
			return;

		if (path.getObject() != &mCurveModel->curve().sourceCurve())
			return;

		QList < int > indices;
		for (int i = 0, len = mCurveModel->curve().pointCount(); i < len; i++)
			indices << i;
		mListenForCurveChanges = false;
		mCurveModel->curve().pointsChanged(indices, true);
		mListenForCurveChanges = true;
	});

	connect(&AppContext::get(), &AppContext::propertyChildInserted, [this](const PropertyPath path, size_t index)
	{
		if (!mCurveModel)
			return;

		if (!mListenForPropertyChanges)
			return;

		if (path.getObject() != &mCurveModel->curve().sourceCurve())
			return;

		mListenForCurveChanges = false;
		mCurveModel->curve().pointsAdded({static_cast<int>(index)});
		mListenForCurveChanges = true;
	});

	connect(&AppContext::get(), &AppContext::propertyChildRemoved, [this](const PropertyPath path, size_t index)
	{
		if (!mCurveModel)
			return;

		if (!mListenForPropertyChanges)
			return;

		if (path.getObject() != &mCurveModel->curve().sourceCurve())
			return;

		mListenForCurveChanges = false;
		mCurveModel->curve().pointsRemoved({static_cast<int>(index)});
		mListenForCurveChanges = true;
	});
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

	connect(&mCurveModel->curve(), &napkin::FCurveAdapter::pointsChanged, [this](QList<int> pointIndexes, bool finished)
	{
		if (!mListenForCurveChanges)
			return;

		// push changes to application
		if (!finished)
			return;

		mListenForPropertyChanges = false;
		for (int i = 0, len = mCurveModel.get()->curve().pointCount(); i < len; i++)
			AppContext::get().propertyValueChanged(mCurveModel->curve().pointPath(i));
		mListenForPropertyChanges = true;
	});

	connect(&mCurveModel->curve(), &napkin::FCurveAdapter::pointsAdded, [this](QList<int> pointIndexes)
	{
		if (!mListenForCurveChanges)
			return;

		mListenForPropertyChanges = false;
		std::sort(pointIndexes.begin(), pointIndexes.end());
		for (int index : pointIndexes)
		{
			AppContext::get().propertyChildInserted(mCurveModel->curve().pointPath(index).getParent(), index);
			AppContext::get().propertyValueChanged(mCurveModel->curve().pointPath(index));
		}
		mListenForPropertyChanges = true;
	});

	connect(&mCurveModel->curve(), &napkin::FCurveAdapter::pointsRemoved, [this](QList<int> pointIndexes)
	{
		mListenForPropertyChanges = false;
		for (int index : nap::qt::reverseSort(pointIndexes))
		{
			AppContext::get().propertyChildRemoved(mCurveModel->curve().pointPath(index).getParent(), index);
			AppContext::get().propertyValueChanged(mCurveModel->curve().pointPath(index));
		}
		mListenForPropertyChanges = true;
	});

	mCurveView.setModel(mCurveModel.get());
}
