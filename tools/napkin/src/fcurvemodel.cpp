/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fcurvemodel.h"
#include "appcontext.h"
#include <napqt/qtutils.h>

using namespace napkin;


const QString FCurveAdapter::name() const
{
	return QString::fromStdString(mCurve.mID);
}

int FCurveAdapter::pointCount() const
{
	return static_cast<int>(mCurve.mPoints.size());
}

void FCurveAdapter::removePoints(const QList<int>& indices)
{
	QList<int> indicesSorted = indices;
	std::sort(indicesSorted.begin(), indicesSorted.end());
	for (int i = indicesSorted.size() - 1; i >= 0; i--)
	{
		mCurve.mPoints.erase(mCurve.mPoints.begin() + indicesSorted[i]);
	}
	mCurve.invalidate();
	pointsRemoved(indicesSorted);
}

void FCurveAdapter::addPoint(qreal time, qreal value)
{
	int index = static_cast<int>(mCurve.mPoints.size());
	mCurve.mPoints.emplace_back(nap::math::FCurvePoint<float, float>({static_cast<float>(time), static_cast<float>(value)}, {-0.1f, 0}, {0.1f, 0}));
	mCurve.invalidate();
	pointsAdded({index});
}

qreal FCurveAdapter::evaluate(qreal t) const
{
	return mCurve.evaluate(static_cast<const float&>(t));
}

const QPointF FCurveAdapter::pos(int pointIndex) const
{
	const auto& pos = mCurve.mPoints[pointIndex].mPos;
	return {pos.mTime, pos.mValue};
}

const QPointF FCurveAdapter::inTangent(int pointIndex) const
{
	const auto& tan = mCurve.mPoints[pointIndex].mInTan;
	return {tan.mTime, tan.mValue};
}

const QPointF FCurveAdapter::outTangent(int pointIndex) const
{
	const auto& tan = mCurve.mPoints[pointIndex].mOutTan;
	return {tan.mTime, tan.mValue};
}

const nap::qt::AbstractCurve::InterpType FCurveAdapter::interpolation(int pointIndex) const
{
	const auto& interp = mCurve.mPoints[pointIndex].mInterp;
	return mInterpMap[interp];
}

void FCurveAdapter::setInterpolation(int pointIndex, const nap::qt::AbstractCurve::InterpType& interp)
{
	nap::math::ECurveInterp destInterp = nap::qt::keyFromValue(mInterpMap, interp, nap::math::ECurveInterp::Bezier);
	mCurve.mPoints[pointIndex].mInterp = destInterp;
	pointsChanged({pointIndex}, true);
}

const bool FCurveAdapter::tangentsAligned(int pointIndex) const
{
	return mCurve.mPoints[pointIndex].mTangentsAligned;
}

void FCurveAdapter::setTangentsAligned(int pointIndex, bool b)
{
	mCurve.mPoints[pointIndex].mTangentsAligned = b;
	pointsChanged({pointIndex}, true);
}


void FCurveAdapter::movePoints(const QMap<int, QPointF>& positions, bool finished)
{
	for (auto it = positions.begin(); it != positions.end(); it++)
	{
		setComplexValue(mCurve.mPoints[it.key()].mPos, it.value());
	}
	pointsChanged(positions.keys(), finished);

	// Points need to be sorted for proper evaluation
	mCurve.invalidate();
}

void FCurveAdapter::setComplexValue(nap::math::FComplex<float, float>& c, const QPointF& p)
{
	c.mTime = static_cast<float>(p.x());
	c.mValue = static_cast<float>(p.y());
}

void FCurveAdapter::moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents, bool finished)
{
	QList<int> changed;
	for (auto it = inTangents.begin(); it != inTangents.end(); it++)
	{
		int index = it.key();
		setComplexValue(mCurve.mPoints[index].mInTan, it.value());

		if (!changed.contains(index))
			changed << index;
	}
	for (auto it = outTangents.begin(); it != outTangents.end(); it++)
	{
		int index = it.key();
		setComplexValue(mCurve.mPoints[index].mOutTan, it.value());
		if (!changed.contains(index))
			changed << index;
	}

	pointsChanged(changed, finished);
}

const PropertyPath FCurveAdapter::pointPath(int pointIndex)
{
	std::string pointPath = QString("Points/%1").arg(pointIndex).toStdString();
	assert(AppContext::isAvailable());
	return {sourceCurve(), nap::rtti::Path::fromString(pointPath), *AppContext::get().getDocument()};
}


const QColor FCurveAdapter::color() const
{
	return QColor::fromRgb(0xFF, 0xFF, 0xFF);
}


FloatFCurveModel::FloatFCurveModel(nap::math::FCurve<float, float>& curve) : nap::qt::AbstractCurveModel(), mCurve(curve) {}

int FloatFCurveModel::curveCount() const
{
	return 1;
}

nap::qt::AbstractCurve* FloatFCurveModel::curve(int index) const
{
	return const_cast<FCurveAdapter*>(&mCurve);
}

void FloatFCurveModel::movePoints(QMap<nap::qt::AbstractCurve*, QMap<int, QPointF>> values)
{
	for (const auto curve : values.keys())
		curve->movePoints(values[curve], true);
}
