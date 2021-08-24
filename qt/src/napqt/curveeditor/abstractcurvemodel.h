/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QList>
#include <QObject>

namespace nap
{
	namespace qt
	{
		class AbstractCurveModel;

		/**
		 * Represents a single curve in the curve editor.
		 * Derive from this class to let your curve become editable in the Curve Editor
		 */
		class AbstractCurve : public QObject
		{
		Q_OBJECT
			Q_ENUMS(InterpType)
		public:
			enum InterpType : int
			{
				Stepped = 0,
				Linear = 1,
				Bezier = 2,
				UserInterp = 0x0100, // Reserved for user interpolation
			};
			Q_ENUM(InterpType)

			explicit AbstractCurve(AbstractCurveModel* parent = nullptr);

			/**
			 * @return The name that will be used in the curve editor for human readability
			 */
			virtual const QString name() const = 0;

			/**
			 * @return The display color of this curve in the editor.
			 */
			virtual const QColor color() const = 0;

			/**
			 * @return The number of points that define this curve.
			 */
			virtual int pointCount() const = 0;

			/**
			 * Evaluate the curve at the specified time.
			 * This evaluation will also be used to display the curve in the editor.
			 *
			 * @param time The time or x-axis value of the curve.
			 * 		This value may extend beyond the first and last curve point's times.
			 * @return The y-axis value of the curve at the provided time.
			 */
			virtual qreal evaluate(qreal time) const = 0;

			/**
			 * @param pointIndex The index of the curve point to get the values of
			 * @return The x-axis (time) and y-axis (value) values of the point at the specified index.
			 */
			virtual const QPointF pos(int pointIndex) const = 0;

			/**
			 * The left tangent of the curve point at the given index, relative to the curve point.
			 * @param pointIndex The index of the curve point.
			 * @return The x (time) and y-axis value of the tangent handle, relative to the curve point.
			 */
			virtual const QPointF inTangent(int pointIndex) const = 0;

			/**
			 * The right tangent of the curve point at the given index, relative to the curve point.
			 * @param pointIndex The index of the curve point.
			 * @return The x (time) and y-axis value of the tangent handle, relative to the curve point.
			 */
			virtual const QPointF outTangent(int pointIndex) const = 0;

			/**
			 * The interpolation method used by the segment adjoining the curve point at the given index.
			 * @param pointIndex The index of the point, left from the curve segment.
			 * @return The type of interpolation being used for the segment
			 */
			virtual const InterpType interpolation(int pointIndex) const = 0;

			/**
			 * Apply the specified interpolation to the segment at the specified index
			 * @param pointIndex The index of the segment
			 * @param interp The interpolation type to be used when evaluating.
			 */
			virtual void setInterpolation(int pointIndex, const InterpType& interp) {}

			/**
			 * Tells the curve editor whether the left and right tangents are aligned,
			 * ensuring C1 continuity (C2 or acceleration continuity is not guaranteed)
			 *
			 * @param pointIndex The index of the point whoms handles to keep aligned during editing
			 * @return true if the tangents are being kept aligned, false otherwise
			 */
			virtual const bool tangentsAligned(int pointIndex) const = 0;

			/**
			 * Tell the curve editor whether the left and right tangents are aligned,
			 * ensuring C1 continuity (C2 or acceleration continuity is not guaranteed)

			 * @param pointIndex The index of the point whoms handles to keep aligned during editing
			 */
			virtual void setTangentsAligned(int pointIndex, bool b) {}

			/**
			 * Move the points at the specified indices to the given positions.
			 * @param positions The indexes and new positions of the points.
			 * @param finished True if a continuous edit action has finished.
			 * 	This function may be called in rapid succession, but always ending on a final value,
			 * 	this is the only time when true is provided.
			 */
			virtual void movePoints(const QMap<int, QPointF>& positions, bool finished) = 0;

			/**
			 * Move the tangents at the specified indices to the given positions.
			 * @param positions The indexes and new tangent positions of the points. relative to point position.
			 * @param finished True if a continuous edit action has finished.
			 * 	This function may be called in rapid succession, but always ending on a final value,
			 * 	this is the only time when true is provided.
			 */
			virtual void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents,
									  bool finished) = 0;

			/**
			 * Remove the points at the specified indices
			 */
			virtual void removePoints(const QList<int>& indices) = 0;

			/**
			 * Insert a point at the specified index, with the given y-axis value
			 * @param time The x-axis value of the new point
			 * @param value The y-axis value of the new point
			 */
			virtual void addPoint(qreal time, qreal value) = 0;

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
			 * @param indices The indices at which the points are being removed
			 */
			void pointsRemoved(QList<int> indices);
		};

		/**
		 * Groups a set of curves for use in a curve editor.
		 * Derive from this class to enable editing a domain-specific curve set representation
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
			 * @param index The indices of the curve to be retrieved
			 * @return The AbstractCurve at the specified index
			 */
			virtual AbstractCurve* curve(int index) const = 0;

			/**
			 * @param curve The curve to find the index of.
			 * @return The index of the provided curve in this model
			 */
			int curveIndex(AbstractCurve* curve) const;

			/**
			 * Move the provided points to the provided values
			 * @param values
			 */
			virtual void movePoints(QMap<AbstractCurve*, QMap<int, QPointF>> values) = 0;

		Q_SIGNALS:
			/**
			 * Invoked when any curves have been inserted into the model.
			 * @param indices The indices of the curves that have been inserted
			 */
			void curvesInserted(QList<int> indices);

			/**
			 * Invoked when any curves are being removed from the model
			 * @param indices The indices of the curves that have been removed
			 */
			void curvesRemoved(QList<int> indices);

			/**
			 * Invoked when any curves have been changed.
			 * @param indices The indices of the curves that have changed
			 */
			void curvesChanged(QList<int> indices);
		};


	} // namespace qt
} // namespace nap
Q_DECLARE_METATYPE(nap::qt::AbstractCurve::InterpType)