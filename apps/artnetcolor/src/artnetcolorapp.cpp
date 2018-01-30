#include "artnetcolorapp.h"
#include "selectcolorcomponent.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <imguiservice.h>
#include <imgui/imgui.h>
#include <mathutils.h>
#include <scene.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArtnetColorApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool ArtnetColorApp::init(utility::ErrorState& error)
	{
		// Create all services
		mRenderService = getCore().getService<RenderService>();
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();
		mArtnetService = getCore().getService<ArtNetService>();
		
		// Load scene
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("artnetcolor.json", error))
			return false; 
		
		// Get important entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mCameraEntity = scene->findEntity("CameraEntity");
		assert(mCameraEntity != nullptr);
		
		mPlaneEntity = scene->findEntity("PlaneEntity");
		assert(mPlaneEntity != nullptr);

		// Get artnet controller
		mArtnetController = mResourceManager->findObject("Universe0");

		// Store all render windows
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");
		mRenderWindow->mWindowEvent.connect(mWindowEventSlot);

		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		// Initialize colors
		std::vector<nap::SelectColorComponentInstance*> comps;
		mPlaneEntity->getComponentsOfType<nap::SelectColorComponentInstance>(comps);
		for (auto& comp : comps)
		{
			mColor.emplace_back(comp->getColor());
			mWhite.emplace_back(comp->getWhite());
		}

		return true;
	}
	
	
	// Called when the window is updating
	void ArtnetColorApp::update(double deltaTime)
	{
		// 1. Show a simple window.
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug".
		{
			ImGui::Text("Hello Sigrid!");
			std::vector<nap::SelectColorComponentInstance*> comps;
			mPlaneEntity->getComponentsOfType<nap::SelectColorComponentInstance>(comps);
			int idx(0);
			for (auto& selector : comps)
			{
				std::string led_color_label = utility::stringFormat("Color %d", idx);
				if (ImGui::ColorEdit3(led_color_label.c_str(), (float*)&mColor[idx].r))
				{
					selector->setColor(mColor[idx]);
				}

				std::string white_color_label = utility::stringFormat("White %d", idx);
				if (ImGui::SliderInt(white_color_label.c_str(), &mWhite[idx], 0, nap::math::max<uint8>()))
				{
					selector->setWhite(static_cast<float>(mWhite[idx]) / static_cast<float>(nap::math::max<uint8>()));
				}

				// show led output colors
				uint8 r, g, b, w;
				selector->getColor(r, g, b, w);
				char ccolor[128];
				snprintf(ccolor, 128, "%d %d %d %d", r, g, b, w);
				std::string dmx_name = utility::stringFormat("LED Color %d", idx);
				ImGui::InputText(dmx_name.c_str(), ccolor, 128, ImGuiInputTextFlags_ReadOnly);

				// show RGB output colors as a combination
				int pr = math::clamp<int>(r + w, 0, math::max<uint8>());
				int pg = math::clamp<int>(g + w, 0, math::max<uint8>());
				int pb = math::clamp<int>(b + w, 0, math::max<uint8>());

				char pxcolor[128];
				snprintf(pxcolor, 128, "%d %d %d", pr, pg, pb);
				std::string pixel_name = utility::stringFormat("Pixel Color %d", idx);
				ImGui::InputText(pixel_name.c_str(), pxcolor, 128, ImGuiInputTextFlags_ReadOnly);

				idx++;
			}
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
	}


	
	// Called when the window is going to render
	void ArtnetColorApp::render()
	{
		mRenderService->destroyGLContextResources({mRenderWindow});
		
		// Activate current window for drawing
		mRenderWindow->makeActive();
		
		// Clear back-buffer
		opengl::RenderTarget& backbuffer = mRenderWindow->getBackbuffer();
		backbuffer.setClearColor(glm::vec4(0.0705f, 0.49f, 0.5647f, 1.0f));
		mRenderService->clearRenderTarget(backbuffer);	
		
		// Render objects
		OrthoCameraComponentInstance& ortho_cam_comp = mCameraEntity->getComponent<nap::OrthoCameraComponentInstance>();
		mRenderService->renderObjects(backbuffer, ortho_cam_comp);

		// Render gui after last gui call
		getCore().getService<IMGuiService>()->draw();

		// Swap backbuffer
		mRenderWindow->swap();
	}
	

	/**
	 * Handles the window event
	 */
	void ArtnetColorApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		if (windowEvent.get_type().is_derived_from(RTTI_OF(WindowResizedEvent)))
		{
			const WindowResizedEvent& res_event = static_cast<const WindowResizedEvent&>(windowEvent);
			nap::TransformComponentInstance& trans_comp = mPlaneEntity->getComponent<nap::TransformComponentInstance>();
			trans_comp.setScale({ res_event.mX, res_event.mY, 1.0 });
			trans_comp.setTranslate({ res_event.mX / 2.0f, res_event.mY / 2.0f, 0.0f });
		}
	}
	
	
	void ArtnetColorApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	void ArtnetColorApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	void ArtnetColorApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen)
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	

	int ArtnetColorApp::shutdown()
	{
		mRenderWindow->mWindowEvent.disconnect(mWindowEventSlot);
		return 0;
	}
}
