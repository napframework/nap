#pragma once

#include <QWidget>
#include <QSplitter>
#include <napqt/filtertreeview.h>
#include <napqt/qtutils.h>
#include "curveview.h"
#include "curvetreemodel.h"

namespace napqt
{



	class CurvePanel : public QWidget
	{
	Q_OBJECT
	public:
		CurvePanel(QWidget* parent = nullptr);

		void setModel(AbstractCurveModel* model);


	private:
		void onTreeDoubleClicked(const QModelIndex& idx);

		QVBoxLayout mLayout;
		QSplitter mSplitter;
		CurveView mCurveView;
		FilterTreeView mTreeView;
		CurveTreeModel mTreeModel;
	};
}