#include "lineblendingapp.h"

// Nap includes
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
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

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

		// Set initial line colors
		mColorOne = mLineEntity->getComponent<LineColorComponentInstance>().getFirstColor();
		mColorTwo = mLineEntity->getComponent<LineColorComponentInstance>().getSecondColor();

		// Set initial line size
		mLineSize = mLineEntity->getComponent<TransformComponentInstance>().getUniformScale();

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
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processEvents(*mRenderWindow, input_router, entities);

		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(utility::getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(ImVec4(clr.getRed(), clr.getGreen(), clr.getBlue(), clr.getAlpha()),
			"left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		// Color
		if (ImGui::CollapsingHeader("Line"))
		{
			if (ImGui::ColorEdit3("Color One", mColorOne.getData()))
			{
				mLineEntity->getComponent<LineColorComponentInstance>().setFirstColor(mColorOne);
			}
			if (ImGui::ColorEdit3("Color Two", mColorTwo.getData()))
			{
				mLineEntity->getComponent<LineColorComponentInstance>().setSecondColor(mColorTwo);
			}
			if (ImGui::SliderFloat("Size", &mLineSize, 0.01f, 1.0f, "%.3f", 2.0f))
			{
				mLineEntity->getComponent<TransformComponentInstance>().setUniformScale(mLineSize);
			}
			if (ImGui::DragFloat2("Position", &(mLinePosition.x), 0.002f, 0.0f, 1.0f))
			{
				// Compute new line position based on size of output frustrum
				// Note that for this to work we just assume the canvas is not rotated and constructed in xy space
				LaserOutputComponentInstance& laser_output = mLaserEntity->getComponent<LaserOutputComponentInstance>();
				float frust_x = laser_output.mProperties.mFrustum.x / 2.0f;
				float frust_y = laser_output.mProperties.mFrustum.y / 2.0f;
				float pos_x = math::lerp<float>(-frust_x, frust_x, mLinePosition.x);
				float pos_y = math::lerp<float>(-frust_y, frust_y, mLinePosition.y);
				TransformComponentInstance& line_xform = mLineEntity->getComponent<TransformComponentInstance>();
				line_xform.setTranslate({ pos_x, pos_y, line_xform.getTranslate().z });
			}
		}
		if (ImGui::CollapsingHeader("Blending"))
		{
			float* blend_speed = &(mLineEntity->getComponent<LineBlendComponentInstance>().mBlendSpeed);
			ImGui::SliderFloat("Blend Speed", blend_speed, 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Noise"))
		{
			LineNoiseComponentInstance& line_mod = mLineEntity->getComponent<LineNoiseComponentInstance>();
			ImGui::SliderFloat("Amplitude", &line_mod.mProperties.mAmplitude, 0.0f, 0.2f, "%.3f", 2.0f);
			ImGui::SliderFloat("Speed", &line_mod.mProperties.mSpeed, 0.0f, 1.0f, "%.3f", 2.0f);
			ImGui::SliderFloat("Offset", &line_mod.mProperties.mOffset, 0.0f, 1.0f);
			ImGui::SliderFloat("Frequency", &line_mod.mProperties.mFrequency, 0.0f, 10.0f, "%.3f", 1.5f);
		}
		ImGui::End();
	}

	
	/**
	 * Render loop is rather straight forward. 
	 * All the objects in the scene are rendered at once including the line + normals and canvas
	 * This demo doesn't require special render steps
	 */
	void LineBlendingApp::render()
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Render all objects in the scene at once
		// This includes the line + normals and the laser canvas
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), mCameraEntity->getComponent<PerspCameraComponentInstance>());

		// Draw gui to screen
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
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