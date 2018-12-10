#include "abstractcurvemodel.h"

using namespace napqt;

AbstractCurve::AbstractCurve(AbstractCurveModel* parent) : QObject(parent)
{

}

AbstractCurveModel::AbstractCurveModel(QObject* parent) : QObject(parent)
{

}

int AbstractCurveModel::curveIndex(AbstractCurve* c) const
{
	for (int i=0, len=curveCount(); i<len; i++)
		if (curve(i) == c)
			return i;
	return -1;
}
