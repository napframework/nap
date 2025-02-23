/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "renderpanel.h"
#include "appcontext.h"
#include "../appletrunner.h"

// External includes
#include <qcolorspace.h>
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
	RenderPanel* RenderPanel::create(napkin::AppletRunner& applet, QWidget* parent, nap::utility::ErrorState& error)
	{
		// SDL window must be created on QT GUI thread
		NAP_ASSERT_MSG(QThread::currentThread() == QCoreApplication::instance()->thread(),
			"SDL event loop must be created and running on the QT GUI thread");

		// Create native window
		QWindow* native_window = new QWindow();

		// Setup QT format (TODO: Use system preferences)
		QSurfaceFormat format;
		format.setColorSpace(QColorSpace(QColorSpace::SRgb));
		native_window->setFormat(format);
		native_window->setSurfaceType(QSurface::VulkanSurface);

		// Create QWidget window container (without parent)
		auto* container = QWidget::createWindowContainer(native_window, parent,
			Qt::Widget | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
		container->setFocusPolicy(Qt::StrongFocus);
		container->setMouseTracking(true);
		container->setMinimumSize({ 256,256 });
		container->setAutoFillBackground(false);
		container->setAttribute(Qt::WA_NoSystemBackground, true);
		container->setAttribute(Qt::WA_UpdatesDisabled, true);

		// Create an SDL window from QT handle ID
		auto id = container->winId(); assert(id != 0);
		if (SDL_SetHintWithPriority(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1", SDL_HINT_OVERRIDE) == SDL_FALSE)
			nap::Logger::warn("Unable to enable '%s'", SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN);

		// TODO: Perform correct nullptr check (0 != nullptr)
		auto sdl_window = SDL_CreateWindowFrom((void*)id);
		if (!error.check(sdl_window != nullptr, "Failed to create window from handle: %s", nap::SDL::getSDLError().c_str()))
		{
			container->setParent(nullptr);
			delete container;
			return nullptr;
		}

		// Make sure that the applet window is created using the given handle
		nap::Core& app_core = applet.getCore();
		auto& factory = app_core.getResourceManager()->getFactory();
		auto obj_creator = std::make_unique<napkin::AppletWindowObjectCreator>(app_core, sdl_window);
		factory.addObjectCreator(std::move(obj_creator));

		// Create and return the new panel
		return new RenderPanel(container, sdl_window, parent, applet);
	}


	RenderPanel::RenderPanel(QWidget* container, SDL_Window* window, QWidget* parent, AppletRunner& applet) :
		mContainer(container), mWindow(window), mApplet(applet), mConverter(window)
	{
		mContainer->installEventFilter(this);
	}


	bool RenderPanel::eventFilter(QObject* obj, QEvent* event)
	{
		// Handle specific events targeting the window
		assert(obj == mContainer);
		switch (event->type())
		{
			// TODO: Figure out why we need to handle these events explicitly ->
			// Without the window is available but drawn (composited) incorrect in Qt (White background)
			case QEvent::Show:
			{
				mApplet.run();
				return true;	
			}
			case QEvent::Hide:
			{
				// Wait for the applet to pause before hiding (and potentially destroying) the window
				auto future_suspend = mApplet.suspend();
				if (future_suspend.valid())
					future_suspend.wait_for(nap::Seconds(5));
				return true;
			}
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseMove:
			case QEvent::Wheel:
			case QEvent::KeyPress:
			case QEvent::KeyRelease:
			{
				auto ptr = mConverter.translateInputEvent(*event);
				assert(ptr != nullptr);
				mApplet.sendEvent(std::move(ptr));
				event->accept();
				return true;
			}
			case QEvent::Resize:
			{
				// Resize window
				auto resize_event = static_cast<QResizeEvent*>(event);
				nap::SDL::setWindowSize(mWindow, { resize_event->size().width(), resize_event->size().height() });
				return true;
			}
			case QEvent::FocusIn:
			case QEvent::FocusOut:
			case QEvent::Paint:
			case QEvent::ParentChange:
			case QEvent::WindowActivate:
			case QEvent::WindowDeactivate:
			case QEvent::ShowToParent:
			case QEvent::HideToParent:
			{
				return true;
			}
			default:
			{
				break;
			}
		}
		return false;
	}
}
