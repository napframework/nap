/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "standardcurve.h"
#include "curvemath.h"

#include <cassert>
#include <QMap>
#include <QtDebug>
#include <napqt/qtutils.h>

using namespace nap::qt;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// StandardCurveModel
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int StandardCurveModel::curveCount() const
{
	return mCurves.size();
}

AbstractCurve* StandardCurveModel::curve(int index) const
{
	return mCurves.at(index);
}

int StandardCurveModel::curveIndex(AbstractCurve* curve) const
{
	for (int i = 0; i < mCurves.size(); i++)
		if (mCurves[i] == curve)
			return i;
	return -1;
}

StandardCurve* StandardCurveModel::addCurve()
{
	auto curve = new StandardCurve(this);
	connect(curve, &AbstractCurve::changed, this, &StandardCurveModel::onCurveChanged);

	int index = mCurves.size();
	mCurves << curve;
	curvesInserted({index});
	return curve;
}

void StandardCurveModel::removeCurve(AbstractCurve* curve)
{
	removeCurve(curveIndex(curve));
}

void StandardCurveModel::removeCurve(int index)
{
	auto idx = static_cast<size_t>(index);
	mCurves.erase(mCurves.begin() + idx);
	curvesRemoved({index});
}

void StandardCurveModel::onCurveChanged(AbstractCurve* curve)
{
	curvesChanged({curveIndex(curve)});
}

void StandardCurveModel::movePoints(QMap<AbstractCurve*, QMap<int, QPointF>> values)
{
	for (auto curve : values.keys()) {
		curve->movePoints(values[curve], true);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// StandardCurve
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StandardCurve::StandardCurve(StandardCurveModel* parent) : AbstractCurve(parent)
{

}

int nap::qt::StandardCurve::pointCount() const
{
	return static_cast<int>(mPoints.size());
}

void StandardCurve::setName(const QString& name)
{
	mName = name;
	changed(this);
}

void StandardCurve::movePoints(const QMap<int, QPointF>& positions, bool finished)
{
	QList<int> indexes;
	auto it = positions.constBegin();
	while (it != positions.constEnd())
	{
		int index = it.key();
		auto pos = it.value();

		mPoints.at(static_cast<unsigned long>(index))->pos = pos;

		indexes << index;
		++it;
	}

	// TODO: Optimize, we only need to update affected points
	mSortedPoints.clear();
	for (auto& p : mPoints)
		mSortedPoints.insert(p->pos.x(), p.get());

	pointsChanged(indexes, false);
}


void StandardCurve::moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents, bool finished)
{
	QList<int> indexes;

	auto it = inTangents.constBegin();
	while (it != inTangents.constEnd())
	{
		int index = it.key();
		auto pos = it.value();
		mPoints.at(static_cast<unsigned long>(index))->inTan = pos;
		if (!indexes.contains(index))
			indexes << index;
		++it;
	}

	it = outTangents.constBegin();
	while (it != outTangents.constEnd())
	{
		int index = it.key();
		auto pos = it.value();
		mPoints.at(static_cast<unsigned long>(index))->outTan = pos;
		if (!indexes.contains(index))
			indexes << index;
		++it;
	}

	pointsChanged(indexes, false);
}

void nap::qt::StandardCurve::addPoint(qreal time, qreal value)
{
	int idx = static_cast<int>(mPoints.size());

	std::unique_ptr<StandardPoint> p = std::make_unique<StandardPoint>(QPointF(time, value));
	auto rawp = p.get();
	mPoints.emplace_back(std::move(p));

	mSortedPoints.insert(time, rawp);

	pointsAdded({idx});
}

void nap::qt::StandardCurve::removePoint(int index)
{
	const auto& point = mPoints.at(static_cast<unsigned long>(index));
	removeFromMapByValue(mSortedPoints, point.get());

	mPoints.erase(mPoints.begin() + index);

	pointsRemoved({index});
}

StandardCurveModel* StandardCurve::model()
{
	return dynamic_cast<StandardCurveModel*>(parent());
}

void StandardCurve::removePoints(const QList<int>& indices)
{
	for (int idx : nap::qt::reverseSort(indices))
	{
		auto p = mPoints.at(static_cast<unsigned long>(idx)).get();
		removeFromMapByValue(mSortedPoints, p);
		mPoints.erase(mPoints.begin() + idx);
	}

	pointsRemoved(indices);
}

qreal StandardCurve::evaluate(qreal time) const
{
	auto first = mSortedPoints.constBegin();
	if (time < first.key())
		return first.value()->pos.y();

	auto last = mSortedPoints.constEnd();
	last--;
	if (time >= last.key())
		return last.value()->pos.y();

	StandardPoint* curr = nullptr;
	StandardPoint* next = nullptr;
	pointsAtTime(time, curr, next);
	assert(curr);
	assert(next);

	auto a = curr->pos;
	auto b = a + curr->outTan;
	auto d = next->pos;
	auto c = d + next->inTan;

	nap::qt::limitOverhangQPoints(a, b, c, d);

	switch (curr->interp)
	{
		case AbstractCurve::InterpType::Bezier:
			return evalCurveSegmentBezier({a, b, c, d}, time);
		case AbstractCurve::InterpType::Linear:
			return evalCurveSegmentLinear({a, b, c, d}, time);
		case AbstractCurve::InterpType::Stepped:
			return evalCurveSegmentStepped({a, b, c, d}, time);
		default:
			assert(false);
	}

	return 0;
}

void StandardCurve::pointsAtTime(qreal time, StandardPoint*& curr, StandardPoint*& next) const
{
	auto lastIt = mSortedPoints.constBegin();
	for (auto it = mSortedPoints.constBegin(); it != mSortedPoints.constEnd(); it++)
	{
		if (it.key() >= time)
		{
			curr = lastIt.value();
			next = it.value();
			return;
		}
		lastIt = it;
	}
	assert(false);
}
const QPointF StandardCurve::pos(int pointIndex) const
{
	return mPoints[pointIndex]->pos;
}

const QPointF StandardCurve::inTangent(int pointIndex) const
{
	return mPoints[pointIndex]->inTan;
}

const QPointF StandardCurve::outTangent(int pointIndex) const
{
	return mPoints[pointIndex]->outTan;
}

const AbstractCurve::InterpType StandardCurve::interpolation(int pointIndex) const
{
	return mPoints[pointIndex]->interp;
}

void StandardCurve::setInterpolation(int pointIndex, const AbstractCurve::InterpType& interp)
{
	mPoints[pointIndex]->interp = interp;
	pointsChanged({pointIndex}, false);
}

const bool StandardCurve::tangentsAligned(int pointIndex) const
{
	return mPoints[pointIndex]->tanAligned;
}

void StandardCurve::setTangentsAligned(int pointIndex, bool b)
{
	mPoints[pointIndex]->tanAligned = b;
	pointsChanged({pointIndex}, false);
}

const QColor StandardCurve::color() const
{
	return Qt::yellow;
}


