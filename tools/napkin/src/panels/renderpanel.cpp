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
#include <SDL_render.h>
#include <SDL_hints.h>
#include <sdlhelpers.h>
#include <QThread>

namespace napkin
{
	RenderPanel* RenderPanel::create(napkin::Applet& applet, QWidget* parent, nap::utility::ErrorState& error)
	{
		// SDL window must be created on QT GUI thread
		NAP_ASSERT_MSG(QThread::currentThread() == QCoreApplication::instance()->thread(),
			"SDL event loop must be created and running on the QT GUI thread");

		// Create native window
		QWindow* native_window = new QWindow();

		// Setup QT format (TODO: Use system preferences)
		QSurfaceFormat format;
		format.setColorSpace(QSurfaceFormat::sRGBColorSpace);
		native_window->setFormat(format);
		native_window->setSurfaceType(QSurface::VulkanSurface);

		// Create QWidget window container (without parent)
		auto* container = QWidget::createWindowContainer(native_window, nullptr);
		container->setFocusPolicy(Qt::StrongFocus);

		// Create an SDL window from QT handle ID
		auto id = container->winId(); assert(id != 0);
		if (SDL_SetHintWithPriority(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1", SDL_HINT_OVERRIDE) == SDL_FALSE)
			nap::Logger::warn("Unable to enable '%s'", SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN);

		// TODO: Perform correct nullptr check (0 != nullptr)
		auto sdl_window = SDL_CreateWindowFrom((void*)id);
		if (!error.check(sdl_window != nullptr, "Failed to create window from handle: %s", nap::SDL::getSDLError().c_str()))
		{
			delete container;
			return nullptr;
		}

		// Make sure that the applet window is created using the given handle
		nap::Core& app_core = applet.getCore();
		auto& factory = app_core.getResourceManager()->getFactory();
		auto obj_creator = std::make_unique<napkin::AppletWindowObjectCreator>(app_core, sdl_window);
		factory.addObjectCreator(std::move(obj_creator));

		// Create and return the new panel
		return new RenderPanel(container, parent);
	}


	bool RenderPanel::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj != mContainer)
			return false;

		switch (event->type())
		{
			case QEvent::Show:
			{
				if (layout() == nullptr)
					setLayout(&mLayout);
				shown(*this);
				return true;
			}
			default:
			{
				break;
			}
		}
		return false;
	}


	RenderPanel::RenderPanel(QWidget* container, QWidget* parent) :
		QWidget(parent), mContainer(container)
	{
		mContainer->setParent(this);
		mContainer->installEventFilter(this);

		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(mContainer);
		installEventFilter(this);
	}

}

