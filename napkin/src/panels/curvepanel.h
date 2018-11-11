#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSettings>

#include <nap/logger.h>

#include <napqt/filtertreeview.h>
#include <napqt/autosettings.h>
#include <napqt/curveeditor/curveview.h>

namespace napkin
{

	class CurvePanel : public QWidget
	{
	Q_OBJECT
	public:
		CurvePanel(QWidget* parent = nullptr);
	private:
		QVBoxLayout mLayout;
		napqt::CurveView mCurveView;
	};

}