/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Mod nap render includes
#include <renderwindow.h>

// External includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <app.h>
#include <entityptr.h>
#include <color.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Very simple demo application that demonstrates how to use a PythonScriptComponent and call it's methods from within C++.
     * A PythonScriptResource is used. This resource references the script 'mycomponent.py', which can be found in the data directory.
     * A python class is defined in this script. This class contains an update(), init() and destroy() method.
     * Additionally it contains a setValue() and getValue() method to access a member.
     * The value of a slider is passed to the setValue() method and the return value of getValue() is displayed in the window.
	 */
	class PythonApp : public App
	{
		RTTI_ENABLE(App)
	public:
		PythonApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;						///< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					///< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							///< Manages all the objects in the scene
		InputService* mInputService = nullptr;							///< Input service for processing input
		IMGuiService* mGuiService = nullptr;							///< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							///< Pointer to the render window
        ObjectPtr<EntityInstance> mPythonEntity = nullptr;              ///< Entity containing the PythonScriptComponent
        float mValue = 0;												///< Value controlled directly by the ImGui slider
		bool mPythonPrint = true;										///< If python prints the value to console
	};
}
