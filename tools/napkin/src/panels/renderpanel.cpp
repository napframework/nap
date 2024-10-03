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

		// Fetch render service
		auto* render_service = AppContext::get().getRenderService();
		assert(render_service != nullptr);

		// Create render window
		assert(mRenderWindow == nullptr && render_service != nullptr);
		auto id = mContainer->winId(); assert(id != 0);
		mRenderWindow = std::make_unique<nap::RenderWindow>(render_service->getCore(), (void*)id);
		mRenderWindow->mID = this->objectName().toStdString() + "_VKWindow";
		nap::utility::ErrorState error;
		if (!mRenderWindow->init(error))
		{
			mRenderWindow = nullptr;
			nap::Logger::error(error.toString());
			return;
		}

		// Add container to layout and set
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(mContainer);
		setLayout(&mLayout);

		// Install listener
		mContainer->installEventFilter(this);
	}


	void RenderPanel::draw()
	{
		// Fetch render service
		auto* render_service = AppContext::get().getRenderService();
		assert(mRenderWindow != nullptr && render_service != nullptr);

		// Clear and draw (TEST)
		auto col = nap::math::random<glm::vec3>({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
		mRenderWindow->setClearColor({col.x, col.y, col.z, 1.0f});
		render_service->beginFrame();
		if (render_service->beginRecording(*mRenderWindow))
		{
			mRenderWindow->beginRendering();
			mRenderWindow->endRendering();
			render_service->endRecording();
		}
		render_service->endFrame();
	}


	bool RenderPanel::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj != mContainer)
			return false;

		switch (event->type())
		{
		case QEvent::Resize:
		{
			QResizeEvent* size_event = static_cast<QResizeEvent*>(event);
			mRenderWindow->setSize({ size_event->size().width(), size_event->size().height() });
			draw();
			return true;
		}
		case QEvent::Show:
		{
			draw();
			return true;
		}
		case QEvent::KeyPress:
		{
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			if (key_event->key() == Qt::Key_Space)
			{
				draw();
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
