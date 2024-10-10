/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "applet.h"

namespace napkin
{
	nap::rtti::ObjectPtr<nap::RenderWindow> Applet::setWindowFromHandle(void* handle, nap::utility::ErrorState& error)
	{
		// Make sure that a new window is created using the given handle
		auto& factory = getCore().getResourceManager()->getFactory();
		auto obj_creator = std::make_unique<napkin::AppletWindowObjectCreator>(getCore(), handle);
		factory.addObjectCreator(std::move(obj_creator));

		// Create and initialize our render window
		mRenderWindow = getCore().getResourceManager()->createObject<nap::RenderWindow>();
		if (!mRenderWindow->init(error))
		{
			mRenderWindow = nullptr;
		}
		return mRenderWindow;
	}
}
