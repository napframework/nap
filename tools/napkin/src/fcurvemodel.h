/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "propertypath.h"

#include <napqt/curveeditor/abstractcurvemodel.h>
#include <fcurve.h>
#include <QtCore/QPointF>
#include <QtCore/qmap.h>

namespace napkin
{

	/**
	 * Adapter between the user interface of the curve editor and the underlying curve data.
	 */
	class FCurveAdapter : public nap::qt::AbstractCurve
	{
	public:
		explicit FCurveAdapter(nap::math::FCurve<float, float>& curve) :
				AbstractCurve(),
				mCurve(curve) {}

		nap::math::FCurve<float, float>& sourceCurve() { return mCurve; }
		const QString name() const override;
		const QColor color() const override;
		int pointCount() const override;
		void removePoints(const QList<int>& indices) override;
		void addPoint(qreal time, qreal value) override;
		qreal evaluate(qreal t) const override;
		const QPointF pos(int pointIndex) const override;
		const QPointF inTangent(int pointIndex) const override;
		const QPointF outTangent(int pointIndex) const override;
		const nap::qt::AbstractCurve::InterpType interpolation(int pointIndex) const override;
		void setInterpolation(int pointIndex, const InterpType& interp) override;
		const bool tangentsAligned(int pointIndex) const override;
		void setTangentsAligned(int pointIndex, bool b) override;
		void movePoints(const QMap<int, QPointF>& positions, bool finished) override;
		void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents,
						  bool finished) override;
		const PropertyPath pointPath(int pointIndex);
	private:
		void setComplexValue(nap::math::FComplex<float, float>& c, const QPointF& p);

		nap::math::FCurve<float, float>& mCurve;
		QMap<nap::math::ECurveInterp, nap::qt::AbstractCurve::InterpType> mInterpMap = {
				{nap::math::ECurveInterp::Stepped, nap::qt::AbstractCurve::InterpType::Stepped},
				{nap::math::ECurveInterp::Linear,  nap::qt::AbstractCurve::InterpType::Linear},
				{nap::math::ECurveInterp::Bezier,  nap::qt::AbstractCurve::InterpType::Bezier},
		};
	};

	class FloatFCurveModel : public nap::qt::AbstractCurveModel
	{
	public:
		explicit FloatFCurveModel(nap::math::FCurve<float, float>& curve);

		int curveCount() const override;
		nap::qt::AbstractCurve* curve(int index) const override;
		FCurveAdapter& curve() { return mCurve; }
		void movePoints(QMap<nap::qt::AbstractCurve*, QMap<int, QPointF>> values) override;
	private:
		FCurveAdapter mCurve;

	};

}