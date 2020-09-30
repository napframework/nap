/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "abstractcurvemodel.h"

using namespace nap::qt;

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
