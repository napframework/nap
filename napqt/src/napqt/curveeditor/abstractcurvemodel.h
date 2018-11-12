#pragma once

#include <QList>
#include <QObject>

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

		virtual const QString name() const = 0;

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
		/**
		 *
		 * @param positions
		 * @param finished
		 */
		virtual void movePoints(const QMap<int, QPointF>& positions, bool finished) = 0;
		virtual void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents, bool finished) = 0;
		virtual void removePoints(const QList<int>& indices) {}
		virtual void addPoint(qreal time, qreal value) {}

	Q_SIGNALS:
		/**
		 * Invoked when this curve changes
		 * @param curve
		 */
		void changed(AbstractCurve* curve);

		/**
		 * Invoked when the points change. This is called continuously during point dragging, so it gets called often.
		 * The last invocation in the edit will set finished to true, so the client may commit the changes.
		 * @param indices The indices of the (unordered) points being changed
		 * @param finished True when changes in rapid succession are done and this is the last change.
		 */
		void pointsChanged(QList<int> indices, bool finished);

		/**
		 * Invoked when one or more points are added to the curve.
		 * @param indices The indices at which the points are inserted
		 */
		void pointsAdded(QList<int> indices);

		/**
		 * Invoked when one or more points are deleted from the curve.
		 * @param indices The incides at which the points are being removed
		 */
		void pointsRemoved(QList<int> indices);
	};

	/**
	 * Groups a set of curves for use in a curve editor
	 */
	class AbstractCurveModel : public QObject
	{
	Q_OBJECT
	public:
		explicit AbstractCurveModel(QObject* parent = nullptr);

		/**
		 * @return The number of curves this model provides
		 */
		virtual int curveCount() const = 0;

		/**
		 * @param index The indes of the curve to be retrieved
		 * @return The AbstractCurve at the specified index
		 */
		virtual AbstractCurve* curve(int index) const = 0;

		/**
		 * @param curve The curve to find the index of.
		 * @return The index of the provided curve in this model
		 */
		int curveIndex(AbstractCurve* curve) const;

	Q_SIGNALS:
		void curvesInserted(QList<int> indices);
		void curvesRemoved(QList<int> indices);
		void curvesChanged(QList<int> indices);
	};


}

Q_DECLARE_METATYPE(napqt::AbstractCurve::InterpType)