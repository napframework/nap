/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <qmap.h>
#include <qpoint.h>

#include "abstractcurvemodel.h"

namespace nap
{
	namespace qt
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

			const QColor color() const override;

			qreal evaluate(qreal time) const override;

			const QPointF pos(int pointIndex) const override;
			const QPointF inTangent(int pointIndex) const override;
			const QPointF outTangent(int pointIndex) const override;
			const InterpType interpolation(int pointIndex) const override;
			void setInterpolation(int pointIndex, const InterpType& interp) override;
			const bool tangentsAligned(int pointIndex) const override;
			void setTangentsAligned(int pointIndex, bool b) override;

			void removePoints(const QList<int>& indices) override;
			void addPoint(qreal time, qreal value) override;
			void removePoint(int index);
			void movePoints(const QMap<int, QPointF>& positions, bool finished) override;
			void moveTangents(const QMap<int, QPointF>& inTangents, const QMap<int, QPointF>& outTangents,
							  bool finished) override;
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
			void movePoints(QMap<AbstractCurve*, QMap<int, QPointF>> values) override;
		private:
			void onCurveChanged(AbstractCurve* curve);
			QList<AbstractCurve*> mCurves;
		};

	} // namespace qt

} // namespace nap
