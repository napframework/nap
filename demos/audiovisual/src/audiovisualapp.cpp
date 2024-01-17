#include "audiovisualapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <renderablemeshcomponent.h>
#include <computecomponent.h>
#include <parametergui.h>
#include <depthsorter.h>
#include <renderdofcomponent.h>
#include <rendertotexturecomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audiovisualApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool audiovisualApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService			= getCore().getService<nap::RenderService>();
		mRenderAdvancedService	= getCore().getService<nap::RenderAdvancedService>();
		mSceneService			= getCore().getService<nap::SceneService>();
		mInputService			= getCore().getService<nap::InputService>();
		mGuiService				= getCore().getService<nap::IMGuiService>();

		// Fetch the resource manager
		mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the render target
		mRenderTarget = mResourceManager->findObject<nap::RenderTarget>("RenderTarget");
		if (!error.check(mRenderTarget != nullptr, "unable to find render target with name: %s", "RenderTarget"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Get the camera entity
		mCameraEntity = mScene->findEntity("CameraEntity");
		if (!error.check(mCameraEntity != nullptr, "unable to find entity with name: %s", "CameraEntity"))
			return false;

		mRenderCameraEntity = mScene->findEntity("RenderCameraEntity");
		if (!error.check(mRenderCameraEntity != nullptr, "unable to find entity with name: %s", "RenderCameraEntity"))
			return false;


		mRenderEntity = mScene->findEntity("RenderEntity");
		if (!error.check(mRenderEntity != nullptr, "unable to find entity with name: %s", "RenderEntity"))
			return false;

		mWorldEntity = mScene->findEntity("WorldEntity");
		if (!error.check(mWorldEntity != nullptr, "unable to find entity with name: %s", "WorldEntity"))
			return false;

		// Cache mask
		mLitRenderMask = mRenderService->findRenderMask("Lit");

		// Reset first frame flag on resource reload
		mResourceManager->mPostResourcesLoadedSignal.connect(mReloadSlot);

		// All done!
		return true;
	}
	
	
	// Update app
	void audiovisualApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// GUI
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::Text(utility::stringFormat("Frametime: %.02fms", deltaTime * 1000.0).c_str());

		for (const auto& gui : mResourceManager->getObjects<ParameterGUI>())
			gui->show(false);

		ImGui::End();
	}
	
	
	// Render app
	void audiovisualApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Compute
		std::vector<ComputeComponentInstance*> compute_comps;
		mWorldEntity->getComponentsOfTypeRecursive<ComputeComponentInstance>(compute_comps);
		if (mRenderService->beginComputeRecording())
		{
			mRenderService->computeObjects(compute_comps);
			mRenderService->endComputeRecording();
		}

		std::vector<RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

		// Headless
		if (mRenderService->beginHeadlessRecording())
		{
			if (mFirstFrame)
			{
				auto cube_targets = mResourceManager->getObjects<CubeRenderTarget>();
				if (!cube_targets.empty())
				{
					assert(cube_targets.front() != nullptr);
					auto target = cube_targets.front();

					for (auto* comp : render_comps)
					{
						if (comp->get_type() != RTTI_OF(RenderableMeshComponentInstance))
							continue;

						auto* resource = comp->getComponent<RenderableMeshComponent>();
						if (resource == nullptr)
							continue;

						if (resource->mID == "RenderStars")
						{
							auto& perp_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();
							target->render([rs = mRenderService, stars = comp](CubeRenderTarget& target, const glm::mat4& projection, const glm::mat4& view) {
								rs->renderObjects(target, projection, view, { stars }, std::bind(&sorter::sortObjectsByDepth, std::placeholders::_1, std::placeholders::_2));
							});
						}
					}
				}
			}

			// Offscreen color pass -> Render all available geometry to the color texture bound to the render target.
			mRenderTarget->beginRendering();
			utility::ErrorState error_state;
			auto lit_comps = mRenderService->filterObjects(render_comps, mLitRenderMask);
			if (!mRenderAdvancedService->pushLights(lit_comps, error_state))
				nap::Logger::error(error_state.toString().c_str());

			auto& cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();
			mRenderService->renderObjects(*mRenderTarget, cam, render_comps);
			mRenderTarget->endRendering();

			// DOF
			mRenderEntity->getComponent<RenderDOFComponentInstance>().draw();
		}
		mRenderService->endHeadlessRecording();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get camera to render with
			auto& cam = mRenderCameraEntity->getComponent<CameraComponentInstance>();

			// Get render chroma component responsible for rendering final texture
			auto* chroma_comp = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("RenderChroma");
			assert(chroma_comp != nullptr);

			// Render composite component
			// The nap::RenderToTextureComponentInstance transforms a plane to match the window dimensions and applies the texture to it.
			mRenderService->renderObjects(*mRenderWindow, cam, { chroma_comp });
		
			// GUI
			if (!mHideGUI)
				mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
		mFirstFrame = false;
	}
	

	void audiovisualApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void audiovisualApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// If we pressed escape, quit the loop
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// f is pressed, toggle full-screen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();

			// h is pressed, toggle GUI
			if (press_event->mKey == nap::EKeyCode::KEY_h)
				mHideGUI = !mHideGUI;
		}
		// Add event, so it can be forwarded on update
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int audiovisualApp::shutdown()
	{
		return 0;
	}

}
