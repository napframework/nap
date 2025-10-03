/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "naputils.h"

// External includes
#include <app.h>
#include <renderwindow.h>
#include <rtti/objectptr.h>
#include <SDL_render.h>
#include <renderservice.h>

namespace napkin
{
	// Forward declares
	class AppletRunner;

	/**
	 * Creates a NAP render window from an SDL window handle
	 */
	class AppletWindowObjectCreator : public nap::rtti::IObjectCreator
	{
	public:
		AppletWindowObjectCreator(nap::Core& core, SDL_Window* windowHandle) :
			mCore(core), mWindowHandle(windowHandle) { }

		/**
		 * @return type to create
		 */
		nap::rtti::TypeInfo getTypeToCreate() const override { return RTTI_OF(nap::RenderWindow); }

		/**
		 * @return Constructs the resource using as a first argument the object stored in this class
		 */
		nap::rtti::Object* create() override { return new nap::RenderWindow(mCore, mWindowHandle); }

	private:
		SDL_Window* mWindowHandle = nullptr;
		nap::Core& mCore;
	};


	/**
	 * Creates a NAP render service configuration from an SDL window handle
	 */
	class RenderServiceObjectCreator : public nap::rtti::IObjectCreator
	{
	public:
		RenderServiceObjectCreator(SDL_Window* windowHandle) :
			mWindowHandle(windowHandle) { }

		/**
		 * @return type to create
		 */
		nap::rtti::TypeInfo getTypeToCreate() const override { return RTTI_OF(nap::RenderServiceConfiguration); }

		/**
		 * @return Constructs the resource using as a first argument the object stored in this class
		 */
		nap::rtti::Object* create() override { return new nap::RenderServiceConfiguration(mWindowHandle); }

	private:
		SDL_Window* mWindowHandle = nullptr;
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
		friend class napkin::AppletRunner;
	public:
		Applet(nap::Core& core) : nap::App(core)	{ }

	protected:
		/**
		 * Changes the current working directory to the data directory that is being edited in Napkin.
		 * Allows data to be de-serialized relative to the napkin project directory, instead of the applet.
		 * The current working directory is active until the handle falls out of scope.
		 * @return Napkin working directory handle
		 */
		napkin::CWDHandle switchWorkingDir() const;

		/**
		 * @return project loaded in editor
		 */
		const nap::ProjectInfo* getEditorInfo() const;
	};
}
