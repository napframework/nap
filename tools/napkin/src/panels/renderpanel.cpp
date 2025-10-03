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
#include <QResizeEvent>
#include <rtti/factory.h>
#include <SDL_render.h>
#include <SDL_hints.h>
#include <sdlhelpers.h>
#include <QThread>

namespace napkin
{
	RenderPanel* RenderPanel::create(napkin::AppletRunner& applet, QWidget& parent, nap::utility::ErrorState& error)
	{
		// SDL window must be created on QT GUI thread
		NAP_ASSERT_MSG(QThread::currentThread() == QCoreApplication::instance()->thread(),
			"SDL event loop must be created and running on the QT GUI thread");

		// Create QWidget window container
		static constexpr int sMinSize = 256;
		auto container = std::make_unique<QWidget>(&parent, Qt::Widget | Qt::FramelessWindowHint);
		container->setFocusPolicy(Qt::StrongFocus);
		container->setMouseTracking(true);
		container->setMinimumSize(sMinSize, sMinSize);
		container->resize(parent.size());
		container->setAutoFillBackground(false);
		container->setAttribute(Qt::WA_UpdatesDisabled, true);
		container->setAttribute(Qt::WA_NativeWindow, true);
		container->setAttribute(Qt::WA_NoSystemBackground, true);

		// Create window properties
		SDL_PropertiesID props = SDL_CreateProperties();
		if(!error.check(props != 0, "Unable to create window properties, '%s'", SDL_GetError()))
			return nullptr;

		// Enable vulkan compatibility
		bool vset = SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
		if (!error.check(vset, "Unable to enable '%s', error: %s", SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, SDL_GetError()))
			return nullptr;

		switch (getVideoDriver())
		{
			case nap::EVideoDriver::Windows:
			{
				auto id = container->winId(); assert(id != 0);
				auto setup = SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, (void*)id);
				error.check(setup, "Unable to enable '%s', error: %s", SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, SDL_GetError());
				break;
			}
			case nap::EVideoDriver::X11:
			{
				auto id = container->winId();
				auto setup = SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER, id);
				error.check(setup, "Unable to enable '%s', error: %s", SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER, SDL_GetError());
				break;
			}
			case nap::EVideoDriver::Wayland:
			{
				// I can't get an embedded widget to work, only a standalone (non-embedded) QWindow, which is of no use.
				// QT reports: 'The cached device pixel ratio value was stale on window expose.  Please file a QTBUG which explains how to reproduce.'
				// 
				// This occurs when using QWidget::createWindowContainer().
				// I am pinning this on QT (for now) and will investigate / try again later, access to the private gui
				// library is also required to acquire the wl surface handle, which is something we should try to avoid.
				//
				// TODO: Add support for embedded applets in wayland (QT)
				error.fail("Wayland video driver currently not supported, use 'xcb' instead");
				break;
			}
			default:
			{
				error.fail("Unsupported applet video-platform: %s",
				           QApplication::platformName().toStdString().c_str());
				break;
			}
		}

		// Ensure platform specific window setup succeeded
		if (error.hasErrors())
			return nullptr;

		// Create SDL window from QWindow
		auto sdl_window = SDL_CreateWindowWithProperties(props);
		if (!error.check(sdl_window != nullptr, "Failed to create window from handle: %s", SDL_GetError()))
			return nullptr;

		// Ensure the applet window is created using the given handle
		nap::Core& app_core = applet.getCore();
		auto& factory = app_core.getResourceManager()->getFactory();
		auto window_creator = std::make_unique<AppletWindowObjectCreator>(app_core, sdl_window);
		factory.addObjectCreator(std::move(window_creator));

		// Ensure the render config is created using the given handle
		auto config_creator = std::make_unique<RenderServiceObjectCreator>(sdl_window);
		factory.addObjectCreator(std::move(config_creator));

		// Create and return the new panel
		return new RenderPanel(container.release(), sdl_window, applet);
	}


	RenderPanel::RenderPanel(QWidget* container, SDL_Window* window, AppletRunner& applet) :
		mContainer(container), mWindow(window), mApplet(applet), mConverter(window, container)
	{
		mContainer->installEventFilter(this);
	}


	bool RenderPanel::eventFilter(QObject* obj, QEvent* event)
	{
		// Handle specific events targeting the window
		assert(obj == mContainer);
		switch (event->type())
		{
			case QEvent::Paint:
			{
				// Override paint
				assert(false);
				return true;
			}
			case QEvent::Show:
			{
				// Run applet when made visible
				mApplet.run();
				auto ptr = mConverter.translateWindowEvent(*event); assert(ptr != nullptr);
				mApplet.sendEvent(std::move(ptr));
				return false;
			}
			case QEvent::Hide:
			{
				// Send hide event
				auto ptr = mConverter.translateWindowEvent(*event); assert(ptr != nullptr);
				mApplet.sendEvent(std::move(ptr));

				// Wait for the applet to pause before hiding (and potentially destroying) the window
				auto future_suspend = mApplet.suspend();
				if (future_suspend.valid())
					future_suspend.wait_for(nap::Seconds(5));
				return false;
			}
			case QEvent::Resize:
			{
				// Send hide event
				auto ptr = mConverter.translateWindowEvent(*event); assert(ptr != nullptr);
				mApplet.sendEvent(std::move(ptr));

				// Explicitly sync window under X11, not required on Windows.
				// TODO: Create an event instead and forward that to the running application
				if (getVideoDriver() == nap::EVideoDriver::X11)
				{
					auto* resize_event = static_cast<QResizeEvent*>(event);
					float ratio = mConverter.getPixelRatio();
					glm::ivec2 sdl_size =
					{
							static_cast<int>(static_cast<float>(resize_event->size().width()) * ratio),
							static_cast<int>(static_cast<float>(resize_event->size().height()) * ratio),
					};

					// Sync window size
					if (!(SDL_SetWindowSize(mWindow, sdl_size.x, sdl_size.y) && SDL_SyncWindow(mWindow)))
						nap::Logger::error(SDL_GetError());
				}
				return false;
			}
			case QEvent::Move:
			{
				auto ptr = mConverter.translateWindowEvent(*event); assert(ptr != nullptr);
				mApplet.sendEvent(std::move(ptr));
				return false;
			}
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseMove:
			case QEvent::Wheel:
			case QEvent::KeyPress:
			case QEvent::KeyRelease:
			{
				auto ptr = mConverter.translateInputEvent(*event); assert(ptr != nullptr);
				mApplet.sendEvent(std::move(ptr));
				event->accept();
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
