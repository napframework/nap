#include "videomodulationapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <orthocameracomponent.h>
#include <perspcameracomponent.h>
#include <rendertexture2d.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <mathutils.h>
#include <selectvideomeshcomponent.h>
#include <audio/component/levelmetercomponent.h>
#include <rendertotexturecomponent.h>
#include <imguiutils.h>
#include <uniforminstance.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoModulationApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	* Initialize all the resources used by this application
	*/
	bool VideoModulationApp::init(utility::ErrorState& error)
	{			
		// Fetch important services
		mRenderService = getCore().getService<RenderService>();
		mInputService = getCore().getService<InputService>();
		mSceneService = getCore().getService<SceneService>();
		mVideoService = getCore().getService<VideoService>();
		mGuiService = getCore().getService<IMGuiService>();

		// Get the resource manager and load our scene / resources
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("videomodulation.json", error))
			return false;
		
		// Get important entities
		rtti::ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mOrthoCameraEntity = scene->findEntity("OrthoCameraEntity");
		mBackgroundEntity = scene->findEntity("BackgroundEntity");
		mVideoEntity = scene->findEntity("VideoEntity");
		mDisplacementEntity = scene->findEntity("DisplacementEntity");
		mPerspCameraEntity = scene->findEntity("PerspCameraEntity");

		// Get render target
		mVideoRenderTarget = mResourceManager->findObject<RenderTarget>("VideoRenderTarget");

		// Find the window we want to render the output to
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");

		// Find the video player
		mVideoPlayer = mResourceManager->findObject<VideoPlayer>("VideoPlayer");
		mCurrentVideo = mVideoPlayer->getIndex();

		// Get current video and mesh index for gui later on
		SelectVideoMeshComponentInstance& mesh_selector = mDisplacementEntity->getComponent<SelectVideoMeshComponentInstance>();
		mCurrentMesh = mesh_selector.getIndex();
		
		// Start video playback
		mVideoPlayer->play();
		
		return true;
	}
	
	
	// Called when the window receives an update
	void VideoModulationApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspCameraEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Update gui components
		updateGui();

		// Position our background based on video ratio / window size
		positionBackground();

		// Push background colors to material
		MaterialInstance& background_material = mBackgroundEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
		background_material.getOrCreateUniform("UBO")->getOrCreateUniform<UniformVec3Instance>("colorOne")->setValue({ mBackgroundColorOne.getRed(), mBackgroundColorOne.getGreen(), mBackgroundColorOne.getBlue() });		
		background_material.getOrCreateUniform("UBO")->getOrCreateUniform<UniformVec3Instance>("colorTwo")->setValue({ mBackgroundColorTwo.getRed(), mBackgroundColorTwo.getGreen(), mBackgroundColorTwo.getBlue() });

		// Push displacement properties to material
		MaterialInstance& displaceme_material = mDisplacementEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
		displaceme_material.getOrCreateUniform("DisplacementUBO")->getOrCreateUniform<UniformFloatInstance>("value")->setValue(mDisplacement);
		displaceme_material.getOrCreateUniform("DisplacementUBO")->getOrCreateUniform<UniformFloatInstance>("randomness")->setValue(mRandomness);
	}
	
	
	/**
	 * We first render the video to it's off screen texture target. The texture it renders to
	 * is linked by both the background and displacement material in JSON.
	 * Next we apply a gray-scale effect to the rendered video texture using a RenderToTextureComponent.
	 * After rendering these passes into their render targets, we render the background using an orthographic camera.
	 * The mesh is rendered on top of the background using a perspective camera.
	 * As a last step the GUI is rendered.
	 */
	void VideoModulationApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Get orthographic camera
		OrthoCameraComponentInstance& ortho_cam = mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>();

		// Start recording into the headless recording buffer.
		if (mRenderService->beginHeadlessRecording())
		{
			// Get objects to render
			std::vector<RenderableComponentInstance*> render_objects;
			render_objects.emplace_back(&mVideoEntity->getComponent<RenderableMeshComponentInstance>());

			// Render the video into the video target
			mVideoRenderTarget->beginRendering();
			mRenderService->renderObjects(*mVideoRenderTarget, ortho_cam, render_objects);
			mVideoRenderTarget->endRendering();

			// Render the gray-scale version of video into the fx target
			RenderToTextureComponentInstance& to_tex_comp = mVideoEntity->getComponent<RenderToTextureComponentInstance>();
			to_tex_comp.draw();

			// Tell the render service we are done rendering into render-targets.
			// The queue is submitted and executed.
			mRenderService->endHeadlessRecording(); 
		}

		// Render everything to screen
		// Stat recording commands for the main window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Clear target and begin render pass
			mRenderWindow->setClearColor({ mClearColor.toVec3(), 1.0f });
			mRenderWindow->beginRendering();

			// Get background plane to render
			std::vector<RenderableComponentInstance*> render_objects;
			render_objects.emplace_back(&mBackgroundEntity->getComponent<RenderableMeshComponentInstance>());

			// Render background plane
			mRenderService->renderObjects(*mRenderWindow, mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>(), render_objects);

			// Render displacement mesh
			render_objects.clear();
			render_objects.emplace_back(&mDisplacementEntity->getComponent<RenderableMeshComponentInstance>());

			// Set camera position in material
			nap::MaterialInstance& material = mDisplacementEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
			UniformVec3Instance* uniform = material.getOrCreateUniform("UBO")->getOrCreateUniform<UniformVec3Instance>("cameraPosition");
			const glm::mat4x4 global_xform = mPerspCameraEntity->getComponent<TransformComponentInstance>().getGlobalTransform();
			uniform->setValue(math::extractPosition(global_xform));

			// Render
			mRenderService->renderObjects(*mRenderWindow, mPerspCameraEntity->getComponent<PerspCameraComponentInstance>(), render_objects);

			// Draw gui
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording into command buffer
			mRenderService->endRecording();
		}

		// Always tell the render engine we are done recording / rendering into this frame
		mRenderService->endFrame();
	}
	
	
	void VideoModulationApp::windowMessageReceived(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}

	
	void VideoModulationApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	int VideoModulationApp::shutdown() 
	{
		return 0;
	}


	// Draw some GUI elements
	void VideoModulationApp::updateGui()
	{
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		
		if (ImGui::CollapsingHeader("Select"))
		{
			SelectVideoMeshComponentInstance& mesh_selector = mDisplacementEntity->getComponent<SelectVideoMeshComponentInstance>();
			if (ImGui::SliderInt("Mesh", &mCurrentMesh, 0, mesh_selector.getCount() - 1))
			{
				mesh_selector.selectMesh(mCurrentMesh);
			}
			if (ImGui::SliderInt("Video", &mCurrentVideo, 0, mVideoPlayer->getCount() - 1))
			{
				utility::ErrorState sel_err;
				if (!mVideoPlayer->selectVideo(mCurrentVideo, sel_err))
					nap::Logger::error(sel_err.toString());
				else
					mVideoPlayer->play();
			}
		}
		if (ImGui::CollapsingHeader("Displacement"))
		{
			ImGui::SliderFloat("Amount", &mDisplacement, 0.0f, 1.0f, "%.3f", 2.0f);
			ImGui::SliderFloat("Random", &mRandomness, 0.0f, 1.0f, "%.3f", 2.25f);
		}
		if (ImGui::CollapsingHeader("Background Colors"))
		{
			ImGui::ColorEdit3("Color One", mBackgroundColorOne.getData());
			ImGui::ColorEdit3("Color Two", mBackgroundColorTwo.getData());
		}
		
		if (ImGui::CollapsingHeader("Playback"))
		{
			float current_time = mVideoPlayer->getCurrentTime();
			if (ImGui::SliderFloat("Current Time", &current_time, 0.0f, mVideoPlayer->getDuration(), "%.3fs", 1.0f))
				mVideoPlayer->seek(current_time);
			ImGui::Text("Total time: %fs", mVideoPlayer->getDuration());
		}
		if (ImGui::CollapsingHeader("Video Texture"))		///< The rendered video texture
		{
			float col_width = ImGui::GetContentRegionAvailWidth();
			nap::Texture2D& video_tex = mVideoRenderTarget->getColorTexture();
			float ratio_video = static_cast<float>(video_tex.getWidth()) / static_cast<float>(video_tex.getHeight());
			ImGui::Image(video_tex, { col_width, col_width / ratio_video });
		}
		if (ImGui::CollapsingHeader("FX Texture"))			///< The post process effect applied to the video texture
		{
			RenderToTextureComponentInstance& fx_comp = mVideoEntity->getComponent<RenderToTextureComponentInstance> ();
			float col_width = ImGui::GetContentRegionAvailWidth();
			nap::Texture2D& output_tex = fx_comp.getOutputTexture();
			float ratio_video = static_cast<float>(output_tex.getWidth()) / static_cast<float>(output_tex.getHeight());
			ImGui::Image(output_tex, { col_width, col_width / ratio_video });
		}

		ImGui::End();
	}


	void VideoModulationApp::positionBackground()
	{
		float window_width = static_cast<float>(mRenderWindow->getWidthPixels());
		float window_heigh = static_cast<float>(mRenderWindow->getHeightPixels());

		// Calculate ratio
		float video_ratio = static_cast<float>(mVideoPlayer->getWidth()) / static_cast<float>(mVideoPlayer->getHeight());
		float window_ratio = window_width / window_heigh;

		glm::vec3 plane_size = { window_width, window_heigh, 1.0 };
		if (window_ratio < video_ratio)
			plane_size.y = plane_size.x / video_ratio;
		else
			plane_size.x = plane_size.y * video_ratio;

		// Calculate plane offset in pixel coordinates
		glm::vec2 offset = { window_width / 2, window_heigh / 2 };

		// Get transform and push
		TransformComponentInstance& transform = mBackgroundEntity->getComponent<TransformComponentInstance>();
		transform.setTranslate(glm::vec3(offset.x, offset.y, 0.0f));
		transform.setScale(plane_size);
	}
}