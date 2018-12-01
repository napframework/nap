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
	/**
	 * A curve editor that allows for editing of a nap::math::FCurve
	 */
	class CurvePanel : public QWidget
	{
	Q_OBJECT
	public:
		explicit CurvePanel(QWidget* parent = nullptr);
		/**
		 * Set or replace the curve that's currently being edited by this editor.
		 * @param curve The curve to be edited, ownership is managed by the client code
		 */
		void editCurve(nap::math::FloatFCurve* curve);

	private:
		QVBoxLayout mLayout;
		napqt::CurveView mCurveView;
		std::shared_ptr<FloatFCurveModel> mCurveModel = nullptr;
		bool mListenForCurveChanges = true;
		bool mListenForPropertyChanges = true;
	};

}