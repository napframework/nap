#pragma once

#include <QList>

namespace napqt
{
	namespace datarole {
		const int TIME = 0;
		const int VALUE = 1;
		const int INTERP = 2;
	}

	class AbstractCurveModel;

	class AbstractCurve : public QObject
	{
	Q_OBJECT
	public:
		enum class InterpolationType
		{
			Stepped, Linear, Bezier
		};

		explicit AbstractCurve(AbstractCurveModel* parent = nullptr);

		virtual int pointCount() const = 0;

		virtual QVariant data(int index, int role) const = 0;
		virtual void setData(int index, int role, QVariant value) = 0;
	Q_SIGNALS:
		void pointsChanged(const QList<int> indices);
		void pointsAdded(const QList<int> indices);
		void pointsRemoved(const QList<int> indices);
	};

	class AbstractCurveModel : public QObject
	{
	Q_OBJECT
	public:
		explicit AbstractCurveModel(QObject* parent);

		virtual int curveCount() const = 0;
		virtual AbstractCurve* curve(int index) = 0;

	Q_SIGNALS:
		void curvesAdded(const QList<int> indices);
		void curvesRemoved(const QList<int> indices);
	};


}

Q_DECLARE_METATYPE(napqt::AbstractCurve::InterpolationType)