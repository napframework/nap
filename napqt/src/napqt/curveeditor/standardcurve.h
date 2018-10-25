#pragma once

#include <memory>

#include "abstractcurvemodel.h"

namespace napqt
{
	class StandardPoint
	{
	public:
		StandardPoint(const QPointF& pos, AbstractCurve::InterpType interp = AbstractCurve::InterpType::Bezier,
				const QPointF& inTan = QPointF(-0.1, 0), const QPointF& outTan = QPointF(0.1, 0))
		: pos(pos), interp(interp), inTan(inTan), outTan(outTan) {}

		QPointF pos;
		QPointF inTan;
		QPointF outTan;
		AbstractCurve::InterpType interp;
	};

	class StandardCurveModel;

	class StandardCurve : public AbstractCurve
	{
	Q_OBJECT
	public:
		explicit StandardCurve(StandardCurveModel* parent = nullptr);

		int pointCount() const override;

		const QString name() const { return mName; }
		void setName(const QString& name);

		QVariant data(int index, int role) const override;
		void setData(int index, int role, QVariant value) override;
		void removePoints(const QList<int>& indices) override;
		void addPoint(qreal time, qreal value) override;
		void removePoint(int index);
		void movePoints(const QMap<int, QPointF>& positions) override;
		void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents) override;
		StandardCurveModel* model();
	private:
		QList<StandardPoint> mPoints;
		QString mName;
	};

	class StandardCurveModel : public AbstractCurveModel
	{
	Q_OBJECT
	public:
		explicit StandardCurveModel(QObject* parent = nullptr) : AbstractCurveModel(parent) {}

		int curveCount() const override;
		AbstractCurve* curve(int index) const override;
		int curveIndex(AbstractCurve* curve) const;
		StandardCurve* addCurve();
		void removeCurve(AbstractCurve* curve);
		void removeCurve(int index);
	private:
		void onCurveChanged(AbstractCurve* curve);
		QList<AbstractCurve*> mCurves;
	};
}