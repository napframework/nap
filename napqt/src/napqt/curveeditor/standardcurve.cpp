
#include "standardcurve.h"

#include <cassert>

using namespace napqt;

namespace napqt
{
	int StandardCurveModel::curveCount() const
	{
		return static_cast<int>(mCurves.size());
	}

	AbstractCurve* StandardCurveModel::curve(int index)
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
}


int napqt::StandardCurve::pointCount() const
{
	return mPoints.size();
}

QVariant napqt::StandardCurve::data(int index, int role) const
{
	auto& p = mPoints.at(index);
	switch (role)
	{
		case datarole::TIME:
			return p.time;
		case datarole::VALUE:
			return p.value;
		case datarole::INTERP:
			return QVariant::fromValue(p.interp);
		default:
			return QVariant();
	}
}

void napqt::StandardCurve::setData(int index, int role, QVariant value)
{
	auto& p = mPoints[index];
	bool ok;
	switch (role)
	{
		case datarole::TIME:
			p.time= value.toReal(&ok);
			assert(ok);
			return;
		case datarole::VALUE:
			p.value = value.toReal(&ok);
			assert(ok);
			return;
		case datarole::INTERP:
			p.interp = value.value<InterpolationType>();
			return;
		default:
			break;
	}
	pointsChanged({index});
}

void napqt::StandardCurve::addPoint(qreal time, qreal value, InterpolationType interp)
{
	mPoints << StandardPoint(time, value, interp);
	pointsAdded({mPoints.size()});
}

void napqt::StandardCurve::removePoint(int index)
{
	mPoints.removeAt(index);
	pointsRemoved({index});
}

StandardCurve::StandardCurve(StandardCurveModel* parent) : AbstractCurve(parent) {}
