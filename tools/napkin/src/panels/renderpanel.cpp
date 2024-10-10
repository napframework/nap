/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "renderpanel.h"
#include "appcontext.h"

// External includes
#include <QSurfaceFormat>
#include <QLayout>
#include <QResizeEvent>
#include <rtti/factory.h>

namespace napkin
{
	RenderPanel::RenderPanel()
	{
		// Create render resources on project load
		connect(&AppContext::get(), &AppContext::projectLoaded, this, &RenderPanel::projectLoaded);
	}


	RenderPanel::~RenderPanel()
	{
		mApplet.abort();
	}


	void RenderPanel::projectLoaded(const nap::ProjectInfo& info)
	{
		// Create if no resources are available
		if (mRenderWindow == nullptr)
		{
			createResources();
		}
	}


	void RenderPanel::createResources()
	{
		// Start initializing the applet (core, services & application)
		auto preview_app = nap::utility::getExecutableDir() + "/resources/apps/renderpreview/app.json";
		nap::utility::ErrorState error;
		mApplet.init(preview_app, std::launch::async);

		// Setup QT format (TODO: Use system preferences)
		QSurfaceFormat format;
		format.setColorSpace(QSurfaceFormat::sRGBColorSpace);
		mNativeWindow.setFormat(format);
		mNativeWindow.setSurfaceType(QSurface::VulkanSurface);

		// Create QWidget window container
		assert(mContainer == nullptr);
		mContainer = QWidget::createWindowContainer(&mNativeWindow, this);
		mContainer->setFocusPolicy(Qt::StrongFocus);

		// Wait for applet to finish initialization -> bail if it failed
		assert(!mInitialized);
		if(!mApplet.initialized())
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Everything initialized correctly, set the render window in the app
		auto id = mContainer->winId(); assert(id != 0);
		mRenderWindow = mApplet.getApplet().setWindowFromHandle((void*)id, error);
		if (mRenderWindow == nullptr)
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Initialization succeeded
		mInitialized = true;

		// Add container to layout and set
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(mContainer);
		setLayout(&mLayout);

		// Install listener
		mContainer->installEventFilter(this);

		// Install timer
		connect(&mTimer, &QTimer::timeout, this, &RenderPanel::timerEvent);
		mTimer.start(20);
	}


	void RenderPanel::timerEvent()
	{ }


	void RenderPanel::abort()
	{
		if (mApplet.running())
			mApplet.abort();
	}


	bool RenderPanel::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj != mContainer)
			return false;

		switch (event->type())
		{
			case QEvent::Show:
			{
				if (!mApplet.running() && mInitialized)
				{
					mApplet.run(std::launch::async, 60);
				}
				return true;
			}
			case QEvent::Close:
			{
				abort();
				return true;
			}
			default:
			{
				break;
			}
		}
		return false;
	}


	void RenderPanel::closeEvent(QCloseEvent* event)
	{
		abort();
	}
}

