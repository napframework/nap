/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "renderpanel.h"
#include "appcontext.h"

// External includes
#include <QSurfaceFormat>
#include <QLayout>

namespace napkin
{
	RenderPanel::RenderPanel()
	{
		// Create render resources on project load
		connect(&AppContext::get(), &AppContext::projectLoaded, this, &RenderPanel::projectLoaded);
	}


	void RenderPanel::projectLoaded(const nap::ProjectInfo& info)
	{
		// Create if no resources are available
		if (mRenderWindow == nullptr)
			createResources();
	}


	void RenderPanel::createResources()
	{
		// Fetch render service
		auto* render_service = AppContext::get().getRenderService();
		assert(render_service != nullptr);

		// Setup format (TODO: Use system preferences)
		QSurfaceFormat format;
		format.setColorSpace(QSurfaceFormat::sRGBColorSpace);
		mNativeWindow.setFormat(format);
		mNativeWindow.setSurfaceType(QSurface::VulkanSurface);

		// Create QWidget window container
		assert(mContainer == nullptr);
		mContainer = QWidget::createWindowContainer(&mNativeWindow, this);
		mContainer->setFocusPolicy(Qt::TabFocus);

		// Set the layout
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(mContainer);
		setLayout(&mLayout);

		// Create render window
		assert(mRenderWindow == nullptr);
		auto id = mContainer->winId(); assert(id != 0);
		mRenderWindow = std::make_unique<nap::RenderWindow>(render_service->getCore(), (void*)id);
		nap::utility::ErrorState error;
		if (!mRenderWindow->init(error))
		{
			nap::Logger::error(error.toString());
		}
	}
}
