/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "appcontext.h"
#include "appletlauncher.h"
#include "apps/renderpreviewapp.h"

// External Includes
#include <QWidget>
#include <QWindow>
#include <QTImer>
#include <QBoxLayout>
#include <renderwindow.h>
#include <guiappeventhandler.h>

namespace napkin
{
	/**
	 * Runs a nap application inside a widget
	 */
	class RenderPanel : public QWidget
	{
		Q_OBJECT
	public:
		// Creates surface and adds it to this widget
		RenderPanel();

		// Free resources
		virtual ~RenderPanel() override;

	protected:
		virtual bool eventFilter(QObject* watched, QEvent* event) override;
		void closeEvent(QCloseEvent* event) override;

	private:
		QWindow mNativeWindow;
		QVBoxLayout mLayout;
		QWidget* mContainer = nullptr;
		nap::ObjectPtr<nap::RenderWindow> mRenderWindow = nullptr;
		void projectLoaded(const nap::ProjectInfo& info);
		void createResources();

		using PreviewApplet = napkin::AppletLauncher<nap::RenderPreviewApp, nap::GUIAppEventHandler>;
		PreviewApplet mApplet;
		bool mInitialized = false;

		QTimer mTimer;
		void timerEvent();
		void abort();
	};
}


