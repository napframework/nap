/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "renderpanel.h"
#include "apps/texturepreviewapp.h"
#include "../appletrunner.h"

namespace napkin
{
	// The applet this panel runs & embeds
	using TextureAppletRunner = TypedAppletRunner<nap::TexturePreviewApp>;

	/**
	 * Allows for previewing material and meshes
	 */
	class TexturePanel : public QWidget
	{
		Q_OBJECT
	public:
		// App to load (relative to executable)
		static constexpr const char* app = "/resources/apps/texturepreview/app.json";

		// Creates the surface and adds it to this widget
		TexturePanel();

		// Ensures applet stops running
		~TexturePanel();

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

		// Called when the embedded render panel is made visible
		void panelShown(napkin::RenderPanel& panel);

	private:
		RenderPanel*			mPanel = nullptr;	//< NAP compatible Qt render window
		TextureAppletRunner		mRunner;			//< Application that is run
		QVBoxLayout				mLayout;			//< Widget layout
		bool					mInitialized;		//< If the panel is initialized

		// Creates the app and links the window
		void init(const nap::ProjectInfo& info);
	};
}

