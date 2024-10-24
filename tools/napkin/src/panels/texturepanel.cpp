/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "texturepanel.h"
#include "../appcontext.h"

namespace napkin
{
	TexturePanel::TexturePanel()
	{
		// Create render resources on project load
		connect(&AppContext::get(), &AppContext::projectLoaded, this, &TexturePanel::init);
	}


	TexturePanel::~TexturePanel()
	{	
		mRunner.abort();
	}


	void TexturePanel::closeEvent(QCloseEvent* event)
	{
		mRunner.abort();
		return QWidget::closeEvent(event);
	}


	void TexturePanel::panelShown(napkin::RenderPanel& panel)
	{
		assert(mPanel != nullptr);
	}


	void TexturePanel::init(const nap::ProjectInfo& info)
	{
		// Signals completion setup resources gui thread
		assert(mPanel == nullptr);

		// Create the window
		nap::utility::ErrorState error;
		mPanel = RenderPanel::create(mRunner, this, error);
		if (mPanel == nullptr)
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Initializing the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		auto init_future = mRunner.start(preview_app, 60);

		// Wait for applet initialization to finish 
		if (init_future.get())
		{
			// Install window into this widget
			assert(layout() == nullptr);
			mLayout.setContentsMargins(0, 0, 0, 0);
			mLayout.addWidget(&mPanel->getWidget());
			setLayout(&mLayout);
		}
	}
}

