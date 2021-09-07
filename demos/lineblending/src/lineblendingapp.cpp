/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "lineblendingapp.h"

// External includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <mathutils.h>
#include <linecolorcomponent.h>
#include <laseroutputcomponent.h>
#include <meshutils.h>
#include <linenoisecomponent.h>
#include <imguiutils.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineBlendingApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool LineBlendingApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService		= getCore().getService<nap::RenderService>();
		mSceneService		= getCore().getService<nap::SceneService>();
		mInputService		= getCore().getService<nap::InputService>();
		mGuiService			= getCore().getService<nap::IMGuiService>();
		mParameterService	= getCore().getService<nap::ParameterService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("lineblending.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Extract the only scene
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Find the entities we're interested in
		mLineEntity = scene->findEntity("Line");
		mCameraEntity = scene->findEntity("Camera");
		mLaserEntity = scene->findEntity("LaserEntity");

		// Create parameter gui
		mParameterGUI = mResourceManager->findObject<ParameterGUI>("ParameterGUI");
		mLineSizeParam = mResourceManager->findObject<ParameterFloat>("line_size");
		mLinePositionParam = mResourceManager->findObject<ParameterVec2>("line_position");

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 * After that we setup the gui.
	 */
	void LineBlendingApp::update(double deltaTime)
	{
		// Select GUI window
		mGuiService->selectWindow(mRenderWindow);

		// Draw some gui elements
		ImGui::Begin("Controls");

		// Show all parameters
		mParameterGUI->show(false);

		// Display some extra info
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(ImVec4(clr.getRed(), clr.getGreen(), clr.getBlue(), clr.getAlpha()),
			"left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::End();

		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Push some parameter settings to components
		// Most custom app components directly reference the parameters
		// except for the transform component
		mLineEntity->getComponent<TransformComponentInstance>().setUniformScale(mLineSizeParam->mValue);
		
		// Compute new line position based on size of output frustrum
		// Note that for this to work we just assume the canvas is not rotated and constructed in xy space
		LaserOutputComponentInstance& laser_output = mLaserEntity->getComponent<LaserOutputComponentInstance>();
		float frust_x = laser_output.mProperties.mFrustum.x / 2.0f;
		float frust_y = laser_output.mProperties.mFrustum.y / 2.0f;
		float pos_x = math::lerp<float>(-frust_x, frust_x, mLinePositionParam->mValue.x);
		float pos_y = math::lerp<float>(-frust_y, frust_y, mLinePositionParam->mValue.y);
		TransformComponentInstance& line_xform = mLineEntity->getComponent<TransformComponentInstance>();
		line_xform.setTranslate({ pos_x, pos_y, line_xform.getTranslate().z });
	}

	
	/**
	 * Render loop is rather straight forward. 
	 * All the objects in the scene are rendered at once including the line + normals and canvas
	 * This demo doesn't require special render steps
	 */
	void LineBlendingApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Start render pass
			mRenderWindow->beginRendering();

			// Render all objects in the scene at once
			// This includes the line + normals and the laser canvas
			mRenderService->renderObjects(*mRenderWindow, mCameraEntity->getComponent<PerspCameraComponentInstance>());

			// Draw gui to screen
			mGuiService->draw();

			// End the render pass
			mRenderWindow->endRendering();

			// End recording framebuffer
			mRenderService->endRecording();
		}

		// Signal end of frame capture operation
		mRenderService->endFrame();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void LineBlendingApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void LineBlendingApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	int LineBlendingApp::shutdown()
	{
		return 0;
	}
}
