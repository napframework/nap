#include "abstractcurve.h"

using namespace napqt;

AbstractCurve::AbstractCurve(AbstractCurveModel* parent) : QObject(parent)
{

}

AbstractCurveModel::AbstractCurveModel(QObject* parent) : QObject(parent)
{

}
