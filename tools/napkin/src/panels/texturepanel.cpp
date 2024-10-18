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
		// Signals completion setup resources gui thread
		assert(mWindow == nullptr);
		std::promise<bool> window_promise;

		// Initializing the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		auto init_future = mRunner.start(preview_app, 60, window_promise.get_future());

		// Wait for the thread to finish initialization and bail if it fails
		if (!init_future.get())
		{
			nap::Logger::error("Unable to initialize preview panel");
			return;
		}

		// Create and set the underlying QWidget render window in the gui thread
		nap::utility::ErrorState error;
		mWindow = RenderPanel::create(mRunner.getApplet(), this, error);
		if (mWindow == nullptr)
		{
			nap::Logger::error(error.toString());
			window_promise.set_value(false);
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

		// Notify runner we're done and it can continue
		window_promise.set_value(true);
	}
}
