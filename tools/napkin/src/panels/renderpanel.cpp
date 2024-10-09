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
		// Setup format (TODO: Use system preferences)
		QSurfaceFormat format;
		format.setColorSpace(QSurfaceFormat::sRGBColorSpace);
		mNativeWindow.setFormat(format);
		mNativeWindow.setSurfaceType(QSurface::VulkanSurface);

		// Create QWidget window container
		assert(mContainer == nullptr);
		mContainer = QWidget::createWindowContainer(&mNativeWindow, this);
		mContainer->setFocusPolicy(Qt::StrongFocus);

		// Initialize applet
		auto preview_app = nap::utility::getExecutableDir() + "/resources/apps/renderpreview/app.json";
		nap::utility::ErrorState error;
		if (!mApplet.init(preview_app, error))
		{
			error.fail("Failed to initialize preview applet!");
			nap::Logger::error(error.toString());
			return;
		}

		// Create render window
		assert(mRenderWindow == nullptr);
		auto id = mContainer->winId(); assert(id != 0);
		mRenderWindow = std::make_unique<nap::RenderWindow>(mApplet.getCore(), (void*)id);
		mRenderWindow->mID = this->objectName().toStdString() + "_VKWindow";
		if (!mRenderWindow->init(error))
		{
			mRenderWindow = nullptr;
			nap::Logger::error(error.toString());
			return;
		}

		// Set the window to use
		mApplet.getApp().setWindow(*mRenderWindow);

		// Add container to layout and set
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(mContainer);
		setLayout(&mLayout);

		// Run the applet (TODO: make it run in the background)
		mApplet.run();

		// Install listener
		mContainer->installEventFilter(this);

		// Intstall timer
		connect(&mTimer, &QTimer::timeout, this, &RenderPanel::timerEvent);
		mTimer.start(20);
	}

	void RenderPanel::draw()
	{
		// Pointer to function used inside update call by core
		auto& app = mApplet.getApp();
		auto& han = mApplet.getHandler();

		// Process and update (core + app)
		han.process();
		std::function<void(double)> update_call = std::bind(&nap::RenderPreviewApp::update, &app, std::placeholders::_1);
		mApplet.getCore().update(update_call);

		// Render
		assert(mRenderWindow != nullptr);
		auto col = nap::math::random<glm::vec3>({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
		mApplet.getApp().render();
	}


	void RenderPanel::timerEvent()
	{
		draw();
	}


	bool RenderPanel::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj != mContainer)
			return false;

		switch (event->type())
		{
		case QEvent::Show:
		{
			return true;
		}
		case QEvent::KeyPress:
		{
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			if (key_event->key() == Qt::Key_Space)
			{
				return true;
			}
			return false;
		}
		default:
		{
			break;
		}
		}
		return false;
	}
}

