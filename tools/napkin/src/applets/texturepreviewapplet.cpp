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
#include <naputils.h>
#include <vulkan/vk_enum_string_helper.h>
#include <pancontroller.h>
#include <imguiutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TexturePreviewApplet)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
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

		// Load API Signature
		mLoadSignature = mResourceManager->findObject<APISignature>(loadCmd);
		if (!error.check(mLoadSignature != nullptr, "Missing '%s' api signature", loadCmd))
			return false;

		// Clear API Signature
		mClearSignature = mResourceManager->findObject<APISignature>(clearCmd);
		if (!error.check(mClearSignature != nullptr, "Missing '%s' api signature", clearCmd))
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

		// Register clear callback
		api_comp.registerCallback(*mClearSignature, mClearRequestedSlot);

		return true;
	}

	
	// Update app
	void TexturePreviewApplet::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;

		// Now forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mOrthoEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Setup GUI
		ImGui::BeginMainMenuBar();
		float bar_height = ImGui::GetWindowHeight();
		float ico_height = bar_height * 0.66f;
		if (ImGui::BeginMenu("Background"))
		{
			ImGui::ColorPicker4("Color", mClearColor.getData());	
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Details", mActiveTexture != nullptr))
		{
			ImGui::PushID(&mActiveTexture);
			texDetail("Width", utility::stringFormat("%d", mActiveTexture->getWidth()), "texel(s)");
			texDetail("Height", utility::stringFormat("%d", mActiveTexture->getHeight()), "texel(s)");
			texDetail("Channels", RTTI_OF(nap::ESurfaceChannels), mActiveTexture->getDescriptor().getChannels());
			texDetail("No. Channels", utility::stringFormat("%d", mActiveTexture->getDescriptor().getNumChannels()));
			texDetail("Surface type", RTTI_OF(nap::ESurfaceDataType), mActiveTexture->getDescriptor().getDataType());
			texDetail("Channel size", utility::stringFormat("%d", mActiveTexture->getDescriptor().getChannelSize()), "byte(s)");
			texDetail("Pixel size", utility::stringFormat("%d", mActiveTexture->getDescriptor().getBytesPerPixel()), "byte(s)");
			texDetail("Surface size", utility::stringFormat("%d", mActiveTexture->getDescriptor().getSizeInBytes()), "byte(s)");
			texDetail("Pitch", utility::stringFormat("%d", mActiveTexture->getDescriptor().getPitch()), "byte(s)");
			texDetail("Layers", utility::stringFormat("%d", mActiveTexture->getLayerCount()));
			texDetail("Mip levels", utility::stringFormat("%d", mActiveTexture->getMipLevels()));
			texDetail("Format", utility::stringFormat(string_VkFormat(mActiveTexture->getFormat())));
			ImGui::PopID();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Applet"))
		{
			ImGui::MenuItem(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
			ImGui::MenuItem(utility::stringFormat("Frametime: %.02fms", deltaTime * 1000.0).c_str());
			ImGui::EndMenu();
		}

		// Add frame icon
		if(mActiveTexture != nullptr &&
			ImGui::ImageButton(mGuiService->getIcon(nap::icon::reload), { ico_height, ico_height }, "Frame"))
			frameTexture();

		ImGui::EndMainMenuBar();
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
		render_window.setClearColor(mClearColor);
		if (mRenderService->beginRecording(render_window))
		{
			// Begin the render pass
			render_window.beginRendering();

			// Get 2D texture and draw
			if (mActiveTexture != nullptr)
			{
				auto& ortho_cam = mOrthoEntity->getComponent<OrthoCameraComponentInstance>();
				auto& tex2d_com = mTextureEntity->getComponent<RenderableMeshComponentInstance>();
				mRenderService->renderObjects(*mRenderWindow, ortho_cam, { &tex2d_com });
			}
			else
			{
				// Otherwise notify user we can select a texture
				Renderable2DTextComponentInstance& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();
				render_text.setLocation({ render_window.getWidthPixels() / 2, render_window.getHeightPixels() / 2 });
				render_text.draw(render_window);
			}

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
		mActiveTexture.reset(nullptr);
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
			error.fail("%s cmd failed", loadCmd);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure there's at least 1 object and it's of type texture
		if (result.mReadObjects.size() == 0 || !result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::Texture2D)))
		{
			nap::Logger::error("%s cmd failed: invalid payload", loadCmd);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...", loadCmd);

		// Init texture relative to project working directory
		{
			napkin::CWDHandle cwd_handle = switchWorkingDir();
			if (!result.mReadObjects[0]->init(error))
			{
				nap::Logger::error(error.toString());
				return;
			}
		}

		// Store and set
		mActiveTexture.reset(static_cast<Texture2D*>(result.mReadObjects[0].release()));

		// Set and frame texture
		auto& render_comp = mTextureEntity->getComponent<nap::RenderableMeshComponentInstance>();
		auto* sampler = render_comp.getMaterialInstance().getOrCreateSampler<Sampler2DInstance>(uniform::texture::sampler::colorTexture);
		assert(sampler != nullptr);
		sampler->setTexture(*mActiveTexture);

		// Reset pan & zoom controls if requested
		auto* frame = apiEvent.getArgumentByName(loadArg2);
		assert(frame != nullptr);
		if (frame->asBool())
		{
			frameTexture();
		}
	}


	void TexturePreviewApplet::onClearRequested(const nap::APIEvent& apiEvent)
	{
		mActiveTexture.reset(nullptr);
	}


	void TexturePreviewApplet::texDetail(std::string&& label, std::string&& value, std::string&& appendix)
	{
		static constexpr float xoff = 125.0f;
		static constexpr float yoff = xoff * 2.0f;
		ImGui::TextColored(mGuiService->getPalette().mFront3Color, label.c_str());
		ImGui::SameLine(xoff);
		ImGui::Text(value.c_str());
		if (!appendix.empty())
		{
			ImGui::SameLine(yoff);
			ImGui::TextColored(mGuiService->getPalette().mFront1Color, appendix.c_str());
		}
	}


	void TexturePreviewApplet::texDetail(std::string&& label, rtti::TypeInfo enumerator, rtti::Variant argument)
	{
		assert(enumerator.is_enumeration());
		texDetail(label.c_str(), enumerator.get_enumeration().value_to_name(argument).data());
	}


	void TexturePreviewApplet::frameTexture()
	{
		// Position texture and reset controller
		assert(mActiveTexture != nullptr);
		auto& pan_controller = mOrthoEntity->getComponent<PanControllerInstance>();
		pan_controller.frameTexture(*mActiveTexture, mTextureEntity->getComponent<TransformComponentInstance>(), 0.9f);
		pan_controller.reset();
	}
}

