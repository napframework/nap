#pragma once

#include <napqt/curveeditor/abstractcurvemodel.h>
#include <fcurve.h>
#include <QtCore/QPointF>
#include <QtCore/qmap.h>
#include "propertypath.h"


namespace napkin
{

	/**
	 * Adapter between the user interface of the curve editor and the underlying curve data.
	 */
	class FCurve : public napqt::AbstractCurve
	{
	public:
		explicit FCurve(nap::math::FunctionCurve<float, float>& curve) :
				AbstractCurve(),
				mCurve(curve) {}

		nap::math::FunctionCurve<float, float>& sourceCurve() { return mCurve; }
		const QString name() const override;
		const QColor color() const override;
		int pointCount() const override;
		void removePoints(const QList<int>& indices) override;
		void addPoint(qreal time, qreal value) override;
		qreal evaluate(qreal t) const override;
		const QPointF pos(int pointIndex) const override;
		const QPointF inTangent(int pointIndex) const override;
		const QPointF outTangent(int pointIndex) const override;
		const napqt::AbstractCurve::InterpType interpolation(int pointIndex) const override;
		void setInterpolation(int pointIndex, const InterpType& interp) override;
		const bool tangentsAligned(int pointIndex) const override;
		void setTangentsAligned(int pointIndex, bool b) override;
		void movePoints(const QMap<int, QPointF>& positions, bool finished) override;
		void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents,
						  bool finished) override;
		const PropertyPath pointPath(int pointIndex);
	private:
		void setComplexValue(nap::math::FComplex<float, float>& c, const QPointF& p);

		nap::math::FunctionCurve<float, float>& mCurve;
		QMap<nap::math::FCurveInterp, napqt::AbstractCurve::InterpType> mInterpMap = {
				{nap::math::FCurveInterp::Stepped, napqt::AbstractCurve::InterpType::Stepped},
				{nap::math::FCurveInterp::Linear,  napqt::AbstractCurve::InterpType::Linear},
				{nap::math::FCurveInterp::Bezier,  napqt::AbstractCurve::InterpType::Bezier},
		};
	};

	class FloatFCurveModel : public napqt::AbstractCurveModel
	{
	public:
		explicit FloatFCurveModel(nap::math::FunctionCurve<float, float>& curve);

		int curveCount() const override;
		napqt::AbstractCurve* curve(int index) const override;
		FCurve& curve() { return mCurve; }
	private:
		FCurve mCurve;

	};

}