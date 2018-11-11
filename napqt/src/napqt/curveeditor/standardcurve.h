#pragma once

#include <memory>

#include "abstractcurvemodel.h"

namespace napqt
{
	class StandardPoint
	{
	public:
		StandardPoint(
				const QPointF& pos,
				AbstractCurve::InterpType interp = AbstractCurve::InterpType::Bezier,
				const QPointF& inTan = QPointF(-0.1, 0),
				const QPointF& outTan = QPointF(0.1, 0),
				bool tanAligned = true)
				: pos(pos),
				  interp(interp),
				  inTan(inTan),
				  outTan(outTan),
				  tanAligned(tanAligned) {}

		QPointF pos;
		QPointF inTan;
		QPointF outTan;
		AbstractCurve::InterpType interp;
		bool tanAligned;
	};

	class StandardCurveModel;

	class StandardCurve : public AbstractCurve
	{
	Q_OBJECT
	public:
		explicit StandardCurve(StandardCurveModel* parent = nullptr);

		int pointCount() const override;

		const QString name() const override { return mName; }
		void setName(const QString& name);
		qreal evaluate(qreal time) const override;

		const QPointF pos(int pointIndex) const override;
		void setPos(int pointIndex, const QPointF& pos) override;
		const QPointF inTangent(int pointIndex) const override;
		void setInTangent(int pointIndex, const QPointF& tan) override;
		const QPointF outTangent(int pointIndex) const override;
		void setOutTangent(int pointIndex, const QPointF& tan) override;
		const InterpType interpolation(int pointIndex) const override;
		void setInterpolation(int pointIndex, const InterpType& interp) override;
		const bool tangentsAligned(int pointIndex) const override;
		void setTangentsAligned(int pointIndex, bool b) override;

		void removePoints(const QList<int>& indices) override;
		void addPoint(qreal time, qreal value) override;
		void removePoint(int index);
		void movePoints(const QMap<int, QPointF>& positions) override;
		void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents) override;
		StandardCurveModel* model();
	private:
		void pointsAtTime(qreal time, StandardPoint*& curr, StandardPoint*& next) const;
		std::vector<std::unique_ptr<StandardPoint>> mPoints;
		QMap<qreal, StandardPoint*> mSortedPoints;
		bool mPointsDirty = true;
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