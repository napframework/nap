#pragma once

#include <QWidget>
#include <QSplitter>
#include <napqt/filtertreeview.h>
#include <napqt/qtutils.h>
#include "curveview.h"

namespace napqt
{
	enum CurveTreeRole {
		ColorRole = Qt::UserRole,

	};

	class CurveTreeModel : public QAbstractItemModel
	{
	Q_OBJECT
	public:
		CurveTreeModel(QObject* parent = nullptr) : QAbstractItemModel(parent) {}

		void setCurveModel(AbstractCurveModel* model);

		AbstractCurve* curveFromIndex(const QModelIndex& idx);

		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

		QModelIndex parent(const QModelIndex& child) const override;

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;
		QVariant data(const QModelIndex& index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex& index) const override;
	private:
		void onCurvesAdded(const QList<int> indexes);
		void onCurvesChanged(const QList<int> indexes);
		void onCurvesRemoved(const QList<int> indexes);

		AbstractCurveModel* mModel = nullptr;
	};

}