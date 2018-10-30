
#include "standardcurve.h"
#include "curvemath.h"

#include <cassert>
#include <QMap>
#include <QtDebug>
#include <napqt/qtutils.h>

using namespace napqt;
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
	curvesAdded({index});
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// StandardCurve
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StandardCurve::StandardCurve(StandardCurveModel* parent) : AbstractCurve(parent)
{

}


int napqt::StandardCurve::pointCount() const
{
	return mPoints.size();
}

void StandardCurve::setName(const QString& name)
{
	mName = name;
	changed(this);
}

QVariant napqt::StandardCurve::data(int index, int role) const
{
	auto& p = *mPoints.at(index).get();
	switch (role)
	{
		case AbstractCurve::PointDataRole::Pos:
			return p.pos;
		case AbstractCurve::PointDataRole::InTangent:
			return p.inTan;
		case AbstractCurve::PointDataRole::OutTangent:
			return p.outTan;
		case AbstractCurve::PointDataRole::Interpolation:
			return QVariant::fromValue(p.interp);
		case AbstractCurve::PointDataRole::AlignedTangents:
			return p.tanAligned;
		default:
			assert(false);
	}
}

void napqt::StandardCurve::setData(int index, int role, QVariant value)
{
	auto p = mPoints.at(index).get();
	switch (role)
	{
		case AbstractCurve::PointDataRole::Pos:
			p->pos = value.toPointF();
			break;
		case AbstractCurve::PointDataRole::InTangent:
			p->inTan = value.toPointF();
			break;
		case AbstractCurve::PointDataRole::OutTangent:
			p->outTan = value.toPointF();
			break;
		case AbstractCurve::PointDataRole::Interpolation:
			p->interp = value.value<InterpType>();
			break;
		case AbstractCurve::PointDataRole::AlignedTangents:
			p->tanAligned = value.toBool();
			break;
		default:
			assert(false);
	}
	pointsChanged({index});
}

void StandardCurve::movePoints(const QMap<int, QPointF>& positions)
{
	QList<int> indexes;
	auto it = positions.constBegin();
	while (it != positions.constEnd())
	{
		int index = it.key();
		auto pos = it.value();

		mPoints.at(index)->pos = pos;

		indexes << index;
		++it;
	}

	// TODO: Optimize, we only need to update affected points
	mSortedPoints.clear();
	for (auto& p : mPoints)
		mSortedPoints.insert(p->pos.x(), p.get());

	pointsChanged(indexes);
}


void StandardCurve::moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents)
{
	QList<int> indexes;

	auto it = inTangents.constBegin();
	while (it != inTangents.constEnd())
	{
		int index = it.key();
		auto pos = it.value();
		mPoints.at(index)->inTan = pos;
		if (!indexes.contains(index))
			indexes << index;
		++it;
	}

	it = outTangents.constBegin();
	while (it != outTangents.constEnd())
	{
		int index = it.key();
		auto pos = it.value();
		mPoints.at(index)->outTan = pos;
		if (!indexes.contains(index))
			indexes << index;
		++it;
	}

	pointsChanged(indexes);
}

void napqt::StandardCurve::addPoint(qreal time, qreal value)
{
	int idx = static_cast<int>(mPoints.size());

	std::unique_ptr<StandardPoint> p = std::make_unique<StandardPoint>(QPointF(time, value));
	auto rawp = p.get();
	mPoints.emplace_back(std::move(p));

	mSortedPoints.insert(time, rawp);

	pointsAdded({idx});
}

void napqt::StandardCurve::removePoint(int index)
{
	const auto& point = mPoints.at(index);
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
	for (int idx : napqt::reverseSort(indices))
	{
		auto p = mPoints.at(idx).get();
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

	napqt::limitOverhangQPoints(a, b, c, d);

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


