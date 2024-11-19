/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "renderpanel.h"
#include "applets/renderpreviewapplet.h"
#include "../appletrunner.h"

// External includes
#include <QLineEdit>
#include <QSpinBox>
#include <apiservice.h>

namespace napkin
{
	// The applet this panel runs & embeds
	using PreviewAppletRunner = TypedAppletRunner<nap::RenderPreviewApplet> ;

	/**
	 * Allows for previewing material and meshes
	 */
	class RenderPreviewPanel : public QWidget
	{
		Q_OBJECT
	public:
		// App to load (relative to executable)
		static constexpr const char* app = "/resources/apps/renderpreview/app.json";

		// Creates the surface and adds it to this widget
		RenderPreviewPanel();

		// Ensures applet stops running
		~RenderPreviewPanel();

		/**
		 * @return if the preview applet is initialized
		 */
		bool initialized() const				{ return mPanel != nullptr; }

		/**
		 * @return if the applet is running
		 */
		bool running() const					{ return mRunner.running(); }

	protected:

		// Called by the main window when the widget is closed
		virtual void closeEvent(QCloseEvent* event) override;

	private:
		QLineEdit				mLineEdit;
		QSpinBox				mSpinbox;
		RenderPanel*			mPanel = nullptr;		//< NAP compatible Qt render window
		PreviewAppletRunner		mRunner;				//< Application that is run
		QVBoxLayout				mMasterLayout;			//< Master widget layout
		QHBoxLayout				mControlLayout;			//< Control widget layout
		bool					mInitialized;			//< If the panel is initialized

		// Creates the app and links the window
		void init(const nap::ProjectInfo& info);

		// Called when the line edit text changes
		void textChanged(const QString& text);

		// Called when the update frequency changes
		void freqChanged(int freq);
	};
}
