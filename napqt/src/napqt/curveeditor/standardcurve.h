#pragma once

#include <memory>

#include "abstractcurve.h"

namespace napqt
{
	class StandardPoint
	{
	public:
		StandardPoint(qreal t, qreal v, AbstractCurve::InterpolationType interp) : time(t), value(v), interp(interp) {}
		StandardPoint(const StandardPoint& other) = default;

		qreal time;
		qreal value;
		AbstractCurve::InterpolationType interp;
	};

	class StandardCurveModel;

	class StandardCurve : public AbstractCurve
	{
	Q_OBJECT
	public:
		explicit StandardCurve(StandardCurveModel* parent = nullptr);

		int pointCount() const override;

		QVariant data(int index, int role) const override;
		void setData(int index, int role, QVariant value) override;

		void addPoint(qreal time, qreal value, InterpolationType interp);
		void removePoint(int index);

	private:
		QList<StandardPoint> mPoints;
	};

	class StandardCurveModel : public AbstractCurveModel
	{
	Q_OBJECT
	public:
		explicit StandardCurveModel(QObject* parent = nullptr) : AbstractCurveModel(parent) {}

		int curveCount() const override;
		AbstractCurve* curve(int index) override;
		int curveIndex(AbstractCurve* curve) const;
		StandardCurve* addCurve();
		void removeCurve(AbstractCurve* curve);
		void removeCurve(int index);
	private:
		QList<AbstractCurve*> mCurves;
	};
}