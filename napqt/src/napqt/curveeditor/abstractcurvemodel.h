#pragma once

#include <QList>

namespace napqt
{


	class AbstractCurveModel;

	/**
	 * Represents a single curve in the curve editor
	 */
	class AbstractCurve : public QObject
	{
	Q_OBJECT
		Q_ENUMS(InterpType)
	public:
		enum InterpType
		{
			Stepped = 0,
			Linear = 1,
			Bezier= 2,

			UserInterp = 0x0100, // Reserved for user interpolation
		};
		Q_ENUM(InterpType)

		explicit AbstractCurve(AbstractCurveModel* parent = nullptr);

		virtual int pointCount() const = 0;

		virtual qreal evaluate(qreal time) const = 0;

		virtual const QPointF pos(int pointIndex) const = 0;
		virtual void setPos(int pointIndex, const QPointF& pos) {}

		virtual const QPointF inTangent(int pointIndex) const = 0;
		virtual void setInTangent(int pointIndex, const QPointF& tan) {}

		virtual const QPointF outTangent(int pointIndex) const = 0;
		virtual void setOutTangent(int pointIndex, const QPointF& tan) {}

		virtual const InterpType interpolation(int pointIndex) const = 0;
		virtual void setInterpolation(int pointIndex, const InterpType& interp) {}

		virtual const bool tangentsAligned(int pointIndex) const = 0;
		virtual void setTangentsAligned(int pointIndex, bool b) {}


		// Point data
		virtual void movePoints(const QMap<int, QPointF>& positions) = 0;
		virtual void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents) = 0;
		virtual void removePoints(const QList<int>& indices) {}
		virtual void addPoint(qreal time, qreal value) {}

	Q_SIGNALS:
		void changed(AbstractCurve* curve);
		void pointsChanged(QList<int> indices);
		void pointsAdded(QList<int> indices);
		void pointsRemoved(QList<int> indices);
	};

	/**
	 * Groups a set of curves for use in a curve editor
	 */
	class AbstractCurveModel : public QObject
	{
	Q_OBJECT
	public:
		explicit AbstractCurveModel(QObject* parent);

		virtual int curveCount() const = 0;
		virtual AbstractCurve* curve(int index) const = 0;
		int curveIndex(AbstractCurve* curve) const;

	Q_SIGNALS:
		void curvesAdded(QList<int> indices);
		void curvesRemoved(QList<int> indices);
		void curvesChanged(QList<int> indices);
	};


}

Q_DECLARE_METATYPE(napqt::AbstractCurve::InterpType)