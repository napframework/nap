#include "texturepreviewapplet.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <apicomponent.h>
#include <rtti/jsonreader.h>
#include <textureshader.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TexturePreviewApplet)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Utility function that fits and centers a plane in a window
	 */
	static void centerPlane(nap::IRenderTarget& window, nap::TransformComponentInstance& planeTransform)
	{
		glm::ivec2 pixel_size = window.getBufferSize();
		float window_width = pixel_size.x;
		float window_heigh = pixel_size.y;

		// Scale of plane is smallest variant of window size
		float scale = 0.0f;
		glm::ivec2 offset(0.0f, 0.0f);
		if (window_width > window_heigh)
		{
			scale = window_heigh;
			offset.x = (window_width - window_heigh) / 2.0f;
		}
		else
		{
			scale = window_width;
			offset.y = (window_heigh - scale) / 2.0f;
		}

		float pos_x = (scale / 2.0f) + offset.x;
		float pos_y = (scale / 2.0f) + offset.y;

		// Push position values to transform
		planeTransform.setTranslate(glm::vec3(pos_x, pos_y, 0.0f));
		planeTransform.setScale(glm::vec3(scale, scale, 1.0f));
	}


	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool TexturePreviewApplet::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService = getCore().getService<nap::RenderService>();
		mSceneService = getCore().getService<nap::SceneService>();
		mInputService = getCore().getService<nap::InputService>();
		mGuiService = getCore().getService<nap::IMGuiService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Fetch render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "Missing 'Window'"))
			return false;

		// API Signature
		mLoadSignature = mResourceManager->findObject<APISignature>(loadCmd1);
		if (!error.check(mLoadSignature != nullptr, "Missing 'SetText' api signature"))
			return false;

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(scene != nullptr, "Missing 'Scene'"))
			return false;

		mTextEntity = scene->findEntity("TextEntity");
		if (!error.check(mTextEntity != nullptr, "Missing 'TextEntity'"))
			return false;

		mAPIEntity = scene->findEntity("APIEntity");
		if (!error.check(mAPIEntity != nullptr, "Missing 'APIEntity'"))
			return false;

		mTextureEntity = scene->findEntity("2DTextureEntity");
		if (!error.check(mTextureEntity != nullptr, "Missing '2DTextureEntity'"))
			return false;

		mOrthoEntity = scene->findEntity("OrthoCameraEntity");
		if (!error.check(mOrthoEntity != nullptr, "Missing 'OrthoCameraEntity'"))
			return false;

		// Register load callback
		auto& api_comp = mAPIEntity->getComponent<nap::APIComponentInstance>();
		api_comp.registerCallback(*mLoadSignature, mLoadRequestedSlot);

		return true;
	}
	
	
	// Update app
	void TexturePreviewApplet::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;

		// Setup GUI
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("Open...");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Info"))
		{
			ImGui::MenuItem(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
			ImGui::MenuItem(utility::stringFormat("Frametime: %.02fms", deltaTime * 1000.0).c_str());
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		// Center plane
		centerPlane(*mRenderWindow, mTextureEntity->getComponent<TransformComponentInstance>());
	}
	
	
	// Render app
	void TexturePreviewApplet::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		nap::RenderWindow& render_window = *mRenderWindow;
		if (mRenderService->beginRecording(render_window))
		{
			// Begin the render pass
			render_window.beginRendering();

			// Get 2D texture and draw
			auto& ortho_cam = mOrthoEntity->getComponent<OrthoCameraComponentInstance>();
			auto& tex2d_com = mTextureEntity->getComponent<RenderableMeshComponentInstance>();
			mRenderService->renderObjects(*mRenderWindow, ortho_cam, { &tex2d_com });

			// Locate component that can render text to screen
			Renderable2DTextComponentInstance& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();
			render_text.setLocation({ render_window.getWidthPixels() / 2, render_window.getHeightPixels() / 2 });
			render_text.draw(render_window);

			// Render gui to screen
			mGuiService->draw();

			// End the render pass
			render_window.endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	

	void TexturePreviewApplet::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void TexturePreviewApplet::inputMessageReceived(InputEventPtr inputEvent)
	{
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int TexturePreviewApplet::shutdown()
	{
		mTexture.reset(nullptr);
		return 0;
	}


	void TexturePreviewApplet::onLoadRequested(const nap::APIEvent& apiEvent)
	{
		auto* data_arg = apiEvent.getArgumentByName(loadArg1);
		assert(data_arg != nullptr);

		// De-serialize JSON
		nap::utility::ErrorState error; nap::DeserializeResult result;
		if (!rtti::deserializeJSON(data_arg->asString(), EPropertyValidationMode::DisallowMissingProperties,
			EPointerPropertyMode::OnlyRawPointers, getCore().getResourceManager()->getFactory(), result, error))
		{
			error.fail("%s cmd failed", loadCmd1);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure there's at least 1 object and it's of type texture
		if (result.mReadObjects.size() == 0 || !result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::Texture2D)))
		{
			nap::Logger::error("%s cmd failed: invalid payload", loadCmd1);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...", loadCmd1);

		// Init texture
		if (!result.mReadObjects[0]->init(error))
			nap::Logger::error(error.toString());

		// Store and set
		mTexture.reset(static_cast<Texture2D*>(result.mReadObjects[0].release()));
		auto& render_comp = mTextureEntity->getComponent<nap::RenderableMeshComponentInstance>();
		auto* sampler = render_comp.getMaterialInstance().getOrCreateSampler<Sampler2DInstance>(uniform::texture::sampler::colorTexture);
		assert(sampler != nullptr);
		sampler->setTexture(*mTexture);
	}
}
