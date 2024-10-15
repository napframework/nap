/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "renderpanel.h"
#include "../appletrunner.h"

namespace napkin
{
	/**
	 * Allows for previewing material and meshes
	 */
	class PreviewPanel : public QWidget
	{
		Q_OBJECT
	public:
		// Creates the surface and adds it to this widget
		PreviewPanel();

		// Ensures applet stops running
		~PreviewPanel();

		// App to load (relative to executable)
		static constexpr const char* app = "/resources/apps/renderpreview/app.json";

	protected:

		// Called by the main window when the widget is closed
		virtual void closeEvent(QCloseEvent* event) override;

	private:
		RenderPanel*			mWindow;		//< Window that is drawn to
		PreviewAppletRunner		mRunner;		//< Application that is run
		QVBoxLayout				mLayout;		//< Widget layout

		// Creates the app and links the window
		void init(const nap::ProjectInfo& info);
	};
}
