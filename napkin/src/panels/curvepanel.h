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
#include <fcurvemodel.h>
#include <propertypath.h>


namespace napkin
{
	class CurvePanel : public QWidget
	{
	Q_OBJECT
	public:
		CurvePanel(QWidget* parent = nullptr);
		void editCurve(nap::math::FloatFCurve* curve);
	private:
		void onCurveUpdated();

		QVBoxLayout mLayout;
		napqt::CurveView mCurveView;
		std::shared_ptr<FloatFCurveModel> mCurveModel = nullptr;
		bool mListenForCurveChanges = true;
	};

}