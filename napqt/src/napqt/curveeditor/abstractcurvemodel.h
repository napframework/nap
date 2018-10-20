#pragma once

#include <QList>

namespace napqt
{
	namespace datarole {
		const int POS = 0;
		const int IN_TAN = 1;
		const int OUT_TAN = 2;
		const int INTERP = 3;
		const int NAME = 4;
	}

	class AbstractCurveModel;

	class AbstractCurve : public QObject
	{
	Q_OBJECT
	public:
		enum class InterpType
		{
			Stepped, Linear, Bezier, BSpline
		};

		explicit AbstractCurve(AbstractCurveModel* parent = nullptr);

		virtual int pointCount() const = 0;

		// Point data
		virtual QVariant data(int index, int role) const = 0;
		virtual void setData(int index, int role, QVariant value) = 0;
		virtual void movePoints(const QMap<int, QPointF>& positions) = 0;
	Q_SIGNALS:
		void changed(AbstractCurve* curve);
		void pointsChanged(QList<int> indices);
		void pointsAdded(const QList<int> indices);
		void pointsRemoved(const QList<int> indices);
	};

	class AbstractCurveModel : public QObject
	{
	Q_OBJECT
	public:
		explicit AbstractCurveModel(QObject* parent);

		virtual int curveCount() const = 0;
		virtual AbstractCurve* curve(int index) const = 0;
		int curveIndex(AbstractCurve* curve) const;

	Q_SIGNALS:
		void curvesAdded(const QList<int> indices);
		void curvesRemoved(const QList<int> indices);
		void curvesChanged(const QList<int> indices);
	};


}

Q_DECLARE_METATYPE(napqt::AbstractCurve::InterpType)