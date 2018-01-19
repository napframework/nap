#include "videomodulationapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <orthocameracomponent.h>
#include <perspcameracomponent.h>
#include <texture2d.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <mathutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool VideoApp::init(utility::ErrorState& error)
	{	
		// Get resource manager service
		mResourceManager = getCore().getResourceManager();
		
		// Create render service
		mRenderService	= getCore().getService<RenderService>();
		mInputService	= getCore().getService<InputService>();
		mSceneService	= getCore().getService<SceneService>();
		mVideoService	= getCore().getService<VideoService>();
		mGuiService		= getCore().getService<IMGuiService>();

		if (!mResourceManager->loadFile("data/videomodulation/videomodulation.json", error))
			return false;
		
		// Get important entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mOrthoCameraEntity = scene->findEntity("OrthoCameraEntity");
		mBackgroundEntity = scene->findEntity("BackgroundEntity");
		mVideoEntity = scene->findEntity("VideoEntity");
		mDisplacementEntity = scene->findEntity("DisplacementEntity");
		mPerspCameraEntity = scene->findEntity("PerspCameraEntity");

		// Get render target
		mVideoRenderTarget = mResourceManager->findObject<RenderTarget>("VideoRenderTarget");

		// Find the window we want to render the output to
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");

		// Position window
		glm::ivec2 screen_size = opengl::getScreenSize(0);
		int offset_x = (screen_size.x - mRenderWindow->getWidth()) / 2;
		int offset_y = (screen_size.y - mRenderWindow->getHeight()) / 2;
		mRenderWindow->setPosition(glm::ivec2(offset_x, offset_y));

		// Collect all video resources and play
		mVideoResource = mResourceManager->findObject<Video>("Video1");
		return true;
	}
	
	
	// Called when the window is updating
	void VideoApp::update(double deltaTime)
	{
		if (!mVideoResource->isPlaying())
		{
			mVideoResource->play();
			mVideoResource->mLoop = true;
		}

		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspCameraEntity.get() };
		mInputService->processEvents(*mRenderWindow, input_router, entities);


		// Update gui components
		updateGui();

		// Position our background based on video ratio / window size
		positionBackground();

		MaterialInstance& video_material = mVideoEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
		video_material.getOrCreateUniform<UniformTexture2D>("yTexture").setTexture(mVideoResource->getYTexture());
		video_material.getOrCreateUniform<UniformTexture2D>("uTexture").setTexture(mVideoResource->getUTexture());
		video_material.getOrCreateUniform<UniformTexture2D>("vTexture").setTexture(mVideoResource->getVTexture());

		MaterialInstance& background_material = mBackgroundEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
		background_material.getOrCreateUniform<UniformVec3>("colorOne").setValue({ mBackgroundColorOne.getRed(), mBackgroundColorOne.getGreen(), mBackgroundColorOne.getBlue() });
		background_material.getOrCreateUniform<UniformVec3>("colorTwo").setValue({ mBackgroundColorTwo.getRed(), mBackgroundColorTwo.getGreen(), mBackgroundColorTwo.getBlue() });
	}
	
	
	// Called when the window is going to render
	void VideoApp::render()
	{
		mRenderService->destroyGLContextResources({mRenderWindow});

		// Make render window active for drawing
		mRenderWindow->makeActive();

		// Get orthographic camera
		OrthoCameraComponentInstance& ortho_cam = mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>();

		// Video render target
		{
			// Clear buffers of video render target
			mRenderService->clearRenderTarget(mVideoRenderTarget->getTarget());
			
			// Get objects to render
			std::vector<RenderableComponentInstance*> render_objects;
			render_objects.emplace_back(&mVideoEntity->getComponent<RenderableMeshComponentInstance>());
			
			// Render
			mRenderService->renderObjects(mVideoRenderTarget->getTarget(), ortho_cam, render_objects);
		}

		// Screen
		{
			// Clear target
			opengl::RenderTarget& render_target = mRenderWindow->getBackbuffer();
			render_target.setClearColor({ mClearColor.getRed(), mClearColor.getGreen(), mClearColor.getBlue(), 1.0 });
			mRenderService->clearRenderTarget(render_target);

			// Get background plane to render
			std::vector<RenderableComponentInstance*> render_objects;
			render_objects.emplace_back(&mBackgroundEntity->getComponent<RenderableMeshComponentInstance>());

			// Render background plane
			mRenderService->renderObjects(render_target, mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>(), render_objects);

			// Render displacement mesh
			render_objects.clear();
			render_objects.emplace_back(&mDisplacementEntity->getComponent<RenderableMeshComponentInstance>());
			
			// Set camera position in material
			nap::MaterialInstance& material = mDisplacementEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
			UniformVec3& uniform = material.getOrCreateUniform<UniformVec3>("cameraPosition");
			const glm::mat4x4 global_xform = mPerspCameraEntity->getComponent<TransformComponentInstance>().getGlobalTransform();
			uniform.setValue(math::extractPosition(global_xform));

			// Render
			mRenderService->renderObjects(render_target, mPerspCameraEntity->getComponent<PerspCameraComponentInstance>(), render_objects);
		}

		// Draw gui
		mGuiService->draw();

		// Swap GPU buffers
		mRenderWindow->swap();
	}
	

	/**
	 * Handles the window event
	 */
	void VideoApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void VideoApp::windowMessageReceived(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}

	
	void VideoApp::inputMessageReceived(InputEventPtr inputEvent)
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

	
	int VideoApp::shutdown() 
	{
		return 0;
	}


	void VideoApp::updateGui()
	{
		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(utility::getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if (ImGui::CollapsingHeader("Background Colors"))
		{
			ImGui::ColorEdit3("Color One", mBackgroundColorOne.getData());
			ImGui::ColorEdit3("Color Two", mBackgroundColorTwo.getData());
		}
		
		ImGui::End();
	}


	void VideoApp::positionBackground()
	{
		float window_width = static_cast<float>(mRenderWindow->getWidth());
		float window_heigh = static_cast<float>(mRenderWindow->getHeight());

		// Calculate ratio
		float video_ratio = static_cast<float>(mVideoResource->getWidth()) / static_cast<float>(mVideoResource->getHeight());
		float window_ratio = window_width / window_heigh;

		glm::vec3 plane_size = { window_width, window_heigh, 1.0 };
		if (window_ratio < video_ratio)
		{
			plane_size.y = plane_size.x / video_ratio;
		}
		else
		{
			plane_size.x = plane_size.y * video_ratio;
		}

		// Calculate plane offset in pixel coordinates
		glm::vec2 offset = { window_width / 2, window_heigh / 2 };

		// Get transform and push
		TransformComponentInstance& transform = mBackgroundEntity->getComponent<TransformComponentInstance>();
		transform.setTranslate(glm::vec3(offset.x, offset.y, 0.0f));
		transform.setScale(plane_size);
	}
}
