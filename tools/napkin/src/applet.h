/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <app.h>
#include <renderwindow.h>
#include <rtti/objectptr.h>

namespace napkin
{
	// Forward declares
	class RenderPanel;

	/**
	 * Creates a NAP render window from a QT window handle
	 */
	class AppletWindowObjectCreator : public nap::rtti::IObjectCreator
	{
	public:
		/**
		*/
		AppletWindowObjectCreator(nap::Core& core, void* windowHandle) :
			mCore(core), mWindowHandle(windowHandle) { }

		/**
		* @return type to create
		*/
		nap::rtti::TypeInfo getTypeToCreate() const override { return RTTI_OF(nap::RenderWindow); }

		/**
		* @return Constructs the resource using as a first argument the object stored in this class
		*/
		virtual nap::rtti::Object* create() override { return new nap::RenderWindow(mCore, mWindowHandle); }

	private:
		void* mWindowHandle = nullptr;
		nap::Core& mCore;
	};


	//////////////////////////////////////////////////////////////////////////
	// Applet
	//////////////////////////////////////////////////////////////////////////

	/**
	 * A NAP application that runs inside a QT Widget.
	 */
	class Applet : public nap::App
	{
		RTTI_ENABLE(nap::App)
		friend class napkin::RenderPanel;
	public:
		Applet(nap::Core& core) : nap::App(core)			{ }

		/**
		 * @return embedded nap render window
		 */
		nap::RenderWindow& getRenderWindow()				{ assert(mRenderWindow != nullptr); return *mRenderWindow; }

	private:
		nap::rtti::ObjectPtr<nap::RenderWindow> mRenderWindow = nullptr;		//< Pointer to the embedded QT render window

		/**
		 * Create and initializes a render window from an external window handle
		 * @param handle the hardware window handle
		 * @param error holds the error if window creation failed
		 */
		nap::rtti::ObjectPtr<nap::RenderWindow> setWindowFromHandle(void* handle, nap::utility::ErrorState& error);
	};
}
