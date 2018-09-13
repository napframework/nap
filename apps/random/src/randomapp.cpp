// Local Includes
#include "randomapp.h"

// External Includes
#include <mathutils.h>
#include <texture2d.h>
#include <rendertexture2d.h>
#include <meshutils.h>
#include <imgui/imgui.h>
#include <imguiservice.h>
#include <utility/stringutils.h>
#include <scene.h>
#include <planemesh.h>
#include <ctime>
#include <chrono>
#include <utility/fileutils.h>
#include <uniforms.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RandomApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	bool RandomApp::init(utility::ErrorState& error)
	{
		// Create services
		mRenderService = getCore().getService<nap::RenderService>();
		mInputService =  getCore().getService<nap::InputService>();
		mSceneService =  getCore().getService<nap::SceneService>();
		mGuiService = getCore().getService<nap::IMGuiService>();

		// Get resource manager and load data
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("random.json", error))
			return false;    

		// Render window and texture target
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		
		// Callback when window event is received
		mRenderWindow->mWindowEvent.connect(std::bind(&RandomApp::handleWindowEvent, this, std::placeholders::_1));

		// All of our entities
		mScene = mResourceManager->findObject<Scene>("Scene");
		
		mCamera = mScene->findEntity("Camera");
		mPlane = mScene->findEntity("Plane");

		// Set render states
		nap::RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::EPolygonMode::Fill;

		return true;
	}

	void RandomApp::update(double deltaTime)
	{
		nap::DefaultInputRouter input_router;
		nap::RenderableMeshComponentInstance& render_plane = mPlane->getComponent<nap::RenderableMeshComponentInstance>();

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCamera.get() };
		mInputService->processEvents(*mRenderWindow, input_router, entities);

		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(utility::getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(ImVec4(clr.getRed(), clr.getGreen(), clr.getBlue(), clr.getAlpha()),
			"left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		
		if (ImGui::CollapsingHeader("Cloud controls"))
		{
			ImGui::SliderFloat("Noise Speed", &mNoiseSpeed, 0.0f, 1.0f);
			ImGui::SliderFloat("Wind Speed", &mWindSpeed, 0.0f, 1.0f);
			ImGui::SliderFloat("Wind Direction", &mWindDirection, 0.0, 360.0);

			nap::UniformFloat& uBrightness = render_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uBrightness");
			ImGui::SliderFloat("Brightness", &(uBrightness.mValue), 0.0f, 1.0f);

			nap::UniformFloat& uContrast = render_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uContrast");
			ImGui::SliderFloat("Contrast", &(uContrast.mValue), 0.0f, 1.0f);
		} 
		ImGui::End();

		nap::UniformVec3& uOffset = render_plane.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset");
		float windDirectionRad = nap::math::radians(mWindDirection);
		float windDistance = mWindSpeed * (float)deltaTime;
		uOffset.mValue.x += cos(windDirectionRad) * windDistance;
		uOffset.mValue.y += sin(windDirectionRad) * windDistance;
		uOffset.mValue.z += mNoiseSpeed * (float)deltaTime;
	}


	void RandomApp::render()
	{
		// Clear all unnecessary gl resources lala
		mRenderService->destroyGLContextResources(std::vector<rtti::ObjectPtr<nap::RenderWindow>>({ mRenderWindow }));

		// Make render window active so we can draw to it
		mRenderWindow->makeActive();

		// Clear
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		nap::RenderableMeshComponentInstance& render_plane = mPlane->getComponent<nap::RenderableMeshComponentInstance>();

		// Find the camera
		nap::PerspCameraComponentInstance& camera = mCamera->getComponent<nap::PerspCameraComponentInstance>();

		// Find the world and add as an object to render
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.emplace_back(&render_plane);

		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, components_to_render);

		// Draw gui
		mGuiService->draw();

		// Swap to front
		mRenderWindow->swap();
	}


	void RandomApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
	}


	int RandomApp::shutdown()
	{
		return 0;
	}


	void RandomApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	void RandomApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
			{
				quit();
				return;
			}

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
				return;
			}
		}

		// Add event to input service for further processing
		mInputService->addEvent(std::move(inputEvent));
	}
}