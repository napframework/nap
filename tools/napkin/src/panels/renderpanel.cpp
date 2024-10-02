/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "renderpanel.h"

// External includes
#include <QSurfaceFormat>
#include <QLayout>

namespace napkin
{
	RenderPanel::RenderPanel()
	{
		// Setup format (TODO: Use system preferences)
		QSurfaceFormat format;
		format.setColorSpace(QSurfaceFormat::sRGBColorSpace);
		format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
		format.setSamples(4);
		mNativeWindow.setFormat(format);

		// Create QWidget window container
		mContainer = QWidget::createWindowContainer(&mNativeWindow, this);
		mContainer->setFocusPolicy(Qt::TabFocus);

		// Add container that holds the windows to widget
		mLayout.addWidget(mContainer);
		mLayout.setContentsMargins(0, 0, 0, 0);
		setLayout(&mLayout);
	}


	RenderPanel::~RenderPanel()
	{
		mNativeWindow.destroy();
	}
}
