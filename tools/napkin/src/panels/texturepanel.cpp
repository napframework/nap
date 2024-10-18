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
		assert(mWindow != nullptr);
	}


	void TexturePanel::init(const nap::ProjectInfo& info)
	{
		assert(mWindow == nullptr);

		// Initializing the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		mRunner.init(preview_app, std::launch::deferred);
		if (!mRunner.initialized())
		{
			nap::Logger::error("Unable to initialize preview panel");
			return;
		}

		// Create the underlying QWidget render window
		nap::utility::ErrorState error;
		mWindow = RenderPanel::create(mRunner.getApplet(), this, error);
		if (mWindow == nullptr)
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Listen to window events
		connect(mWindow, &RenderPanel::shown, this, &TexturePanel::panelShown);

		// Tell event loop to forward events to this applet
		auto* event_loop = AppContext::get().getEventLoop();
		assert(event_loop != nullptr);
		//event_loop->setApplet(mRunner);

		// Install window into this widget
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(mWindow);
		setLayout(&mLayout);

		// Start running the application (threaded)
		mRunner.run(std::launch::async, 60);
	}
}
