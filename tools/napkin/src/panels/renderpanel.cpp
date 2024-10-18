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
	RenderPanel* RenderPanel::create(napkin::Applet& applet, QWidget* parent, nap::utility::ErrorState& error)
	{
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

		// Everything initialized correctly, set the render window in the app
		auto id = container->winId(); assert(id != 0);
		auto render_window = applet.setWindowFromHandle((void*)id, error);
		if (render_window == nullptr)
		{
			delete container;
			return nullptr;
		}

		// Create and return the new panel
		return new RenderPanel(container, render_window, parent);
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


	RenderPanel::RenderPanel(QWidget* container, nap::rtti::ObjectPtr<nap::RenderWindow> window, QWidget* parent) :
		QWidget(parent), mContainer(container), mRenderWindow(window)
	{
		mContainer->setParent(this);
		mContainer->installEventFilter(this);

		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(mContainer);
		installEventFilter(this);
	}

}

