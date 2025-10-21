/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imguiservice.h"
#include "imguifont.h"
#include "imgui/imgui_impl_vulkan.h"

// External Includes
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <nap/core.h>
#include <color.h>
#include <SDL_clipboard.h>
#include <SDL_properties.h>
#include <SDL_mouse.h>
#include <SDL_keyboard.h>
#include <nap/logger.h>
#include <materialcommon.h>
#include <sdlhelpers.h>
#include <nap/modulemanager.h>

RTTI_BEGIN_CLASS(nap::IMGuiServiceConfiguration)
	RTTI_PROPERTY("ColorScheme",		&nap::IMGuiServiceConfiguration::mColorScheme,		nap::rtti::EPropertyMetaData::Default,	"Global GUI color scheme to use")
	RTTI_PROPERTY("FontSize",			&nap::IMGuiServiceConfiguration::mFontSize,			nap::rtti::EPropertyMetaData::Default,	"Global GUI font size")
	RTTI_PROPERTY("GlobalScale",		&nap::IMGuiServiceConfiguration::mScale,			nap::rtti::EPropertyMetaData::Default,	"Global GUI scaling factor")
	RTTI_PROPERTY_FILELINK("FontFile",	&nap::IMGuiServiceConfiguration::mFontFile,			nap::rtti::EPropertyMetaData::Default,nap::rtti::EPropertyFileType::Font, "Font override")
	RTTI_PROPERTY("FontSampling",		&nap::IMGuiServiceConfiguration::mFontOversampling, nap::rtti::EPropertyMetaData::Default,	"Horiontal and Vertical GUI oversampling factor")
	RTTI_PROPERTY("FontSpacing",		&nap::IMGuiServiceConfiguration::mFontSpacing,		nap::rtti::EPropertyMetaData::Default,	"Extra horizontal spacing (in pixels) between glyphs")
	RTTI_PROPERTY("Colors",				&nap::IMGuiServiceConfiguration::mCustomColors,		nap::rtti::EPropertyMetaData::Default,	"Colors to use when ColorScheme is set to 'Custom'")
	RTTI_PROPERTY("Style",				&nap::IMGuiServiceConfiguration::mStyle,			nap::rtti::EPropertyMetaData::Default,	"Global GUI styling options")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMGuiService, "Manages the global GUI state")
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// Define thread local ImGUI context (Important!)
// Ensures the current enabled context is local to the thread accessing it
// Without thread local access the context is shared among gui instances
//////////////////////////////////////////////////////////////////////////
thread_local ImGuiContext* ImGuiTLS = nullptr;

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Icons
	//////////////////////////////////////////////////////////////////////////

	namespace icon
	{
		static const std::vector<std::string>& getDefaults()
		{
			const static std::vector<std::string> map =
			{
				icon::save,
				icon::saveAs,
				icon::cancel,
				icon::del,
				icon::file,
				icon::help,
				icon::settings,
				icon::ok,
				icon::reload,
				icon::load,
				icon::info,
				icon::warning,
				icon::error,
				icon::copy,
				icon::paste,
				icon::insert,
				icon::edit,
				icon::remove,
				icon::add,
				icon::change,
				icon::subtract,
				icon::frame
			};
			return map;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Static / Local methods
	//////////////////////////////////////////////////////////////////////////

	static void checkVKResult(VkResult err)
	{
		if (err == 0) return;
		assert(false);
		nap::Logger::error("ImGUI Vulkan Error! code: %d", err);
	}


	static VkCommandBuffer beginSingleTimeCommands(RenderService& renderService)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = renderService.getCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		VkResult err = vkAllocateCommandBuffers(renderService.getDevice(), &allocInfo, &commandBuffer);
		checkVKResult(err);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		checkVKResult(err);

		return commandBuffer;
	}


	static void endSingleTimeCommands(RenderService& renderService, VkCommandBuffer commandBuffer)
	{
		VkResult err = vkEndCommandBuffer(commandBuffer);
		checkVKResult(err);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		err = vkQueueSubmit(renderService.getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		checkVKResult(err);

		err = vkQueueWaitIdle(renderService.getQueue());
		checkVKResult(err);

		vkFreeCommandBuffers(renderService.getDevice(), renderService.getCommandPool(), 1, &commandBuffer);
	}


	static void createDeviceObjects(RenderService& renderService, VkSampler& sampler, VkDescriptorSetLayout& layout)
	{
		VkResult err;

		// Create sampler
		VkSamplerCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter = VK_FILTER_LINEAR;
		info.minFilter = VK_FILTER_LINEAR;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		info.minLod = -1000;
		info.maxLod = 1000;
		info.maxAnisotropy = 1.0f;
		err = vkCreateSampler(renderService.getDevice(), &info, nullptr, &sampler);
		checkVKResult(err);

		// Create a descriptor set layout for the images we want to display
		VkDescriptorSetLayoutBinding binding[1] = {};
		binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding[0].descriptorCount = 1;
		binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding[0].pImmutableSamplers = VK_NULL_HANDLE;

		VkDescriptorSetLayoutCreateInfo set_info = {};
		set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set_info.bindingCount = 1;
		set_info.pBindings = binding;
		err = vkCreateDescriptorSetLayout(renderService.getDevice(), &set_info, nullptr, &layout);
		checkVKResult(err);
	}


	static void destroyDeviceObjects(RenderService& renderService, VkSampler sampler, VkDescriptorSetLayout layout)
	{
		if (layout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(renderService.getDevice(), layout, nullptr);
		}

		if (sampler != VK_NULL_HANDLE) {
			vkDestroySampler(renderService.getDevice(), sampler, nullptr);
		}
	}


	// Create descriptor pool for imgui vulkan implementation, allows creation / allocation right resources on that side
	static void createFontDescriptorPool(RenderService& renderService, VkDescriptorPool& pool)
	{
		VkDescriptorPoolSize pool_size = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)(1) };

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &pool_size;
		poolInfo.maxSets = 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VkResult result = vkCreateDescriptorPool(renderService.getDevice(), &poolInfo, nullptr, &pool);
		assert(result == VK_SUCCESS);
	}


	static void destroyFontDescriptorPool(RenderService& renderService, VkDescriptorPool pool)
	{
		vkDestroyDescriptorPool(renderService.getDevice(), pool, nullptr);
	}


	static const char* getClipboardText(void*)
	{
		return SDL_GetClipboardText();
	}


	static void setClipboardText(void*, const char* text)
	{
		SDL_SetClipboardText(text);
	}


	static ImGuiContext* createContext(const IMGuiServiceConfiguration& configuration, ImFontAtlas& fontAtlas, const ImGuiStyle& style, const std::string& iniFile)
	{
		// Create ImGUI context
		ImGuiContext* new_context = ImGui::CreateContext(&fontAtlas);
		ImGuiContext* cur_context = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(new_context);

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = (int)EKeyCode::KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = (int)EKeyCode::KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = (int)EKeyCode::KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = (int)EKeyCode::KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = (int)EKeyCode::KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = (int)EKeyCode::KEY_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = (int)EKeyCode::KEY_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = (int)EKeyCode::KEY_HOME;
		io.KeyMap[ImGuiKey_End] = (int)EKeyCode::KEY_END;
		io.KeyMap[ImGuiKey_Delete] = (int)EKeyCode::KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = (int)EKeyCode::KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = (int)EKeyCode::KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = (int)EKeyCode::KEY_RETURN;
		io.KeyMap[ImGuiKey_Escape] = (int)EKeyCode::KEY_ESCAPE;
		io.KeyMap[ImGuiKey_KeyPadEnter] = (int)EKeyCode::KEY_KP_ENTER;
		io.KeyMap[ImGuiKey_A] = (int)EKeyCode::KEY_a;
		io.KeyMap[ImGuiKey_C] = (int)EKeyCode::KEY_c;
		io.KeyMap[ImGuiKey_V] = (int)EKeyCode::KEY_v;
		io.KeyMap[ImGuiKey_X] = (int)EKeyCode::KEY_x;
		io.KeyMap[ImGuiKey_Y] = (int)EKeyCode::KEY_y;
		io.KeyMap[ImGuiKey_Z] = (int)EKeyCode::KEY_z;

		// Set callbacks
		io.SetClipboardTextFn = setClipboardText;
		io.GetClipboardTextFn = getClipboardText;
		io.ClipboardUserData = NULL;

		// Set style
		ImGui::GetStyle() = style;

		// Attempt to load .ini settings
		ImGui::LoadIniSettingsFromDisk(iniFile.c_str());

		// Pop context
		ImGui::SetCurrentContext(cur_context);

		return new_context;
	}


	static std::unique_ptr<ImFontAtlas> createFontAtlas(float fontSize, const glm::ivec2& fontSampling, float fontSpacing, const char* fontFile = nullptr)
	{
		// Create font atlas
		std::unique_ptr<ImFontAtlas> new_atlas = std::make_unique<ImFontAtlas>();
		ImFontConfig font_config;
		font_config.OversampleH = fontSampling.x;
		font_config.OversampleV = fontSampling.y;
		font_config.GlyphExtraSpacing.x = fontSpacing;

		// Add font, scale based on main dpi (TODO: Make Monitor Aware)
		float font_size = math::floor(fontSize);
		if (fontFile != nullptr)
		{
			new_atlas->AddFontFromFileTTF(fontFile, font_size, &font_config);
		}
		else
		{
			new_atlas->AddFontFromMemoryCompressedTTF(manropeMediumData, manropeMediumSize, font_size, &font_config);
		}
		return new_atlas;
	}


	static void setGuiWindow(SDL_Window* window)
	{
#ifdef _WIN32
		auto hwnd = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = hwnd;
#else
		(void)window;
#endif
	}

	// Global key modifiers
	static constexpr int modControl = 0;
	static constexpr int modAlt = 1;
	static constexpr int modShift = 2;
	static constexpr int modNone = -1;

	static int getModKeyIndex(nap::EKeyCode key)
	{
		switch (key)
		{
		case EKeyCode::KEY_LCTRL:
		case EKeyCode::KEY_RCTRL:
			return modControl;
		case EKeyCode::KEY_LALT:
		case EKeyCode::KEY_RALT:
			return modAlt;
		case EKeyCode::KEY_LSHIFT:
		case EKeyCode::KEY_RSHIFT:
			return modShift;
		default:
			break;
		}
		return modNone;
	}


	//////////////////////////////////////////////////////////////////////////
	// Service Methods
	//////////////////////////////////////////////////////////////////////////

	IMGuiService::IMGuiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{}


	void IMGuiService::draw()
	{
		// Get window we're drawing in
		RenderWindow* current_window = mRenderService->getCurrentRenderWindow();
		if (current_window == nullptr)
		{
			assert(false);
			nap::Logger::warn("IMGuiService: calling draw without active render window");
			return;
		}

		// Switch context based on currently activated render window
		const auto it = mContexts.find(current_window);
		assert(it != mContexts.end());
		ImGui::SetCurrentContext(it->second->mContext);

		// Render GUI
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData
		(
			ImGui::GetDrawData(),
			it->second->mContext,
			mRenderService->getCurrentCommandBuffer(),
			current_window->getRenderPass(),
			current_window->getSampleCount()
		);
	}


	void IMGuiService::selectWindow(nap::ResourcePtr<RenderWindow> window)
	{
		// Select ImGUI context
		const auto it = mContexts.find(window.get());
		assert(it != mContexts.end());
		ImGui::SetCurrentContext(it->second->mContext);

		// Update SDL Window information
		setGuiWindow(it->first->getNativeWindow());
	}


	ImGuiContext* IMGuiService::getContext(nap::ResourcePtr<RenderWindow> window)
	{
		// Select ImGUI context
		const auto it = mContexts.find(window.get());
		assert(it != mContexts.end());
		return it->second->mContext;
	}


	ImGuiContext* IMGuiService::findContext(int windowID)
	{
		auto* window = mRenderService->findWindow(windowID);
		if (window != nullptr)
		{
			const auto it = mContexts.find(window);
			assert(it != mContexts.end());
			return it->second->mContext;
		}
		return nullptr;
	}


	float IMGuiService::getScale(const ImGuiContext* context) const
	{
		// Check if service doesn't already exist
		const auto& found_it = std::find_if(mContexts.begin(), mContexts.end(), [&context](const auto& it)
		{
			return it.second->mContext == context;
		});
		assert(found_it != mContexts.end());
		return found_it->second->mScale;
	}


	float IMGuiService::getScale() const
	{
		ImGuiContext* context = ImGui::GetCurrentContext();
		return context != nullptr ? getScale(context) : -1.0f;
	}


	ImGuiContext* IMGuiService::processInputEvent(const InputEvent& event)
	{
		// Check if it's a window input event
		if (!event.get_type().is_derived_from(RTTI_OF(WindowInputEvent)))
			return nullptr;

		// Find window associated with event
		const auto& input_event = static_cast<const WindowInputEvent&>(event);
		RenderWindow* window = mRenderService->findWindow(input_event.mWindow);
		assert(window != nullptr);

		// Get context for window
		auto context = mContexts.find(window);
		assert(context != mContexts.end());

		// Activate ImGUI Context
		ImGui::SetCurrentContext(context->second->mContext);

		// Pointer press event
		if (event.get_type().is_derived_from(RTTI_OF(PointerEvent)))
			handlePointerEvent(static_cast<const PointerEvent&>(event), *context->second);

		// Key event
		else if (event.get_type().is_derived_from(RTTI_OF(KeyEvent)))
			handleKeyEvent(static_cast<const KeyEvent&>(event), *context->second);

		// Touch event
		else if (event.get_type().is_derived_from(RTTI_OF(TouchEvent)))
			handleTouchEvent(static_cast<const TouchEvent&>(event), *context->second);

		// Text input event
		else if (event.get_type().is_derived_from(RTTI_OF(TextInputEvent)))
		{
			ImGuiIO& io = ImGui::GetIO();
			const auto& press_event = static_cast<const TextInputEvent&>(event);
			io.AddInputCharactersUTF8(press_event.mText.c_str());
		}

		// Mouse wheel event
		else if (event.get_type().is_derived_from(RTTI_OF(MouseWheelEvent)))
		{
			const auto& wheel_event = static_cast<const MouseWheelEvent&>(event);
			int delta = wheel_event.mY;
			context->second->mMouseWheel = delta > 0 ? 1.0f : -1.0f;
		}

		return context->second->mContext;
	}


	bool IMGuiService::isCapturingKeyboard(ImGuiContext* context)
	{
		// Get the interface
		ImGui::SetCurrentContext(context);
		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureKeyboard;
	}


	bool IMGuiService::isCapturingMouse(ImGuiContext* context)
	{
		// Get the interface
		ImGui::SetCurrentContext(context);
		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureMouse;
	}


	void IMGuiService::addInputCharachter(ImGuiContext* context, nap::uint character)
	{
		ImGui::SetCurrentContext(context);
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(character);
	}


	ImTextureID IMGuiService::getTextureHandle(const nap::Texture2D& texture) const
	{
		// Check if the texture has been requested before
		auto it = mDescriptors.find(&texture);
		if (it != mDescriptors.end())
			return (ImTextureID)(it->second);

		// Allocate new description set
		VkDescriptorSet descriptor_set = mAllocator->allocate(mDescriptorSetLayout, 0, 0, 1);

		// Update description set
		VkDescriptorImageInfo desc_image[1] = {};
		desc_image[0].sampler = mSampler;
		desc_image[0].imageView = texture.getHandle().getView();
		desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Create write structure and update  descriptor set
		VkWriteDescriptorSet write_desc[1] = {};
		write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc[0].dstSet = descriptor_set;
		write_desc[0].descriptorCount = 1;
		write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_desc[0].pImageInfo = desc_image;

		vkUpdateDescriptorSets(mRenderService->getDevice(), 1, write_desc, 0, NULL);
		mDescriptors.emplace(std::make_pair(&texture, descriptor_set));
		return (ImTextureID)(descriptor_set);
	}


	nap::Icon& IMGuiService::getIcon(std::string&& name)
	{
		return *mIcons[name];
	}


	bool IMGuiService::loadIcon(const std::string& name, const nap::Module& module, utility::ErrorState& error)
	{
		// Find path to asset
		auto icon_path = module.findAsset(name);
		if (!error.check(!icon_path.empty(), "%s: Unable to find icon %s", module.getName().c_str(), name.c_str()))
			return false;

		// Create and initialize icon
		auto new_icon = std::make_unique<Icon>(*this, icon_path);
		new_icon->mInvert = mColorPalette->mInvertIcon;
		if (!new_icon->init(error))
			return false;

		auto ret = mIcons.emplace(std::make_pair(name, std::move(new_icon)));
		if (!error.check(ret.second, "Icon duplication, %s already found in: %s",
			name.c_str(), utility::forceSeparator(ret.first->second->getPath()).c_str()))
			return false;
		return true;
	}


	const nap::gui::ColorPalette& IMGuiService::getPalette() const
	{
		assert(mColorPalette != nullptr);
		return *mColorPalette;
	}


	bool IMGuiService::init(utility::ErrorState& error)
	{
		// Get our renderer
		mRenderService = getCore().getService<nap::RenderService>();
		assert(mRenderService != nullptr);

		// Get configuration
		mConfiguration = getConfiguration<IMGuiServiceConfiguration>();
		assert(mConfiguration != nullptr);

		// Register to window added / removed signals
		mRenderService->windowAdded.connect(mWindowAddedSlot);
		mRenderService->windowRemoved.connect(mWindowRemovedSlot);

		// Global GUI & DPI scale
		mGuiScale = math::max<float>(mConfiguration->mScale, math::epsilon<float>());

		// Get palette associated with scheme
		nap::gui::registerCustomPalette(mConfiguration->mCustomColors);
		mColorPalette = gui::getPalette(mConfiguration->mColorScheme);
		assert(mColorPalette != nullptr);

		// Load all the default icons, bail if any of them fails to load
		const auto& default_icons = icon::getDefaults();
		for (const auto& icon_name : default_icons)
		{
			if (!loadIcon(icon_name, getModule(), error))
				return false;
		}
		return true;
	}


	void IMGuiService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
		dependencies.emplace_back(RTTI_OF(RenderService));
	}


	void IMGuiService::update(double deltaTime)
	{
		for (auto& context : mContexts)
		{
			// Signal the beginning of a new frame to the im-gui backend
			newFrame(*context.first, *context.second, deltaTime);

			// Signal the beginning of a new frame to the vulkan backend
			ImGui_ImplVulkan_NewFrame();
		}
	};


	void IMGuiService::postUpdate(double deltaTime)
	{
		for (auto& context : mContexts)
		{
			ImGui::SetCurrentContext(context.second->mContext);
			ImGui::EndFrame();
		}
	}


	void IMGuiService::preShutdown()
	{
		// Ensure ini directory exists
		std::string dir = getCore().getProjectInfo()->getIniDir();
		if (!utility::ensureDirExists(dir))
		{
			nap::Logger::warn("Unable to write %s file(s) to directory: %s", projectinfo::iniExtension, dir.c_str());
			return;
		}

		// Save imGUI .ini settings to disk, split by window
		for (const auto& context : mContexts)
		{
			ImGui::SetCurrentContext(context.second->mContext);
			ImGui::SaveIniSettingsToDisk(getIniFilePath(context.first->mID).c_str());
		}
	}


	void IMGuiService::shutdown()
	{
		if (mFontAtlas != nullptr)
		{
			// Destroy vulkan resources
			ImGui_ImplVulkan_Shutdown();
			destroyDeviceObjects(*mRenderService, mSampler, mDescriptorSetLayout);
			destroyFontDescriptorPool(*mRenderService, mDescriptorPool);
			mAllocator.reset(nullptr);
		}

		// Destroy imgui contexts
		mContexts.clear();

		// Destroy icons
		mIcons.clear();
	}


	void IMGuiService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<IconObjectCreator>(*this));
	}


	void IMGuiService::onWindowAdded(RenderWindow& window)
	{
		// Create new context
		assert(mContexts.find(&window) == mContexts.end());

		// Create font atlas if not present, this also means it's the first time a window is added
		// In that case we create the necessary GUI vulkan resources
		ImGuiContext* new_context = nullptr;
		if (mFontAtlas == nullptr)
		{
			// Calculate reference scaling factor based on reference display
			// TODO: a different font atlas should be created for every variation in scale,
			// TODO: and dynamically bound every frame, based on active window scaling factor.
			mReferenceScale = math::max<float>(window.getDisplayScale(), 1.0f);

			// Create atlas, scale based on dpi of main monitor
			const char* font_file = mConfiguration->mFontFile.empty() ? nullptr : mConfiguration->mFontFile.c_str();
			float font_size = mConfiguration->mFontSize * mReferenceScale * mGuiScale;
			mFontAtlas = createFontAtlas(font_size, mConfiguration->mFontOversampling, mConfiguration->mFontSpacing, font_file);

			// Create style
			assert(mConfiguration != nullptr);
			assert(mColorPalette  != nullptr);
			mStyle = gui::createStyle(*mColorPalette, mConfiguration->mStyle);

			// Create context using font & style
			new_context = createContext(*getConfiguration<IMGuiServiceConfiguration>(), *mFontAtlas, *mStyle, getIniFilePath(window.mID));

			// Create all vulkan required resources
			createVulkanResources(window);
		}
		else
		{
			// New context for window
			new_context = createContext(*getConfiguration<IMGuiServiceConfiguration>(), *mFontAtlas, *mStyle, getIniFilePath(window.mID));
		}

		// Add context, set display index & push scale accordingly
		auto it = mContexts.emplace(std::make_pair(&window, std::make_unique<GUIContext>(new_context, mStyle.get())));
		pushScale(*it.first->second, window);

		// Connect so we can listen to window events such as move
		window.mWindowEvent.connect(mWindowEventSlot);
	}


	void IMGuiService::onWindowRemoved(RenderWindow& window)
	{
		// Disconnect
		window.mWindowEvent.disconnect(mWindowEventSlot);

		// Find context
		auto it = mContexts.find(&window);
		assert(it != mContexts.end());

		// Remove on vulkan side and erase
		ImGui_ImplVulkan_RemoveContext(it->second->mContext);
		mContexts.erase(it);
	}


	void IMGuiService::onWindowEvent(const WindowEvent& windowEvent)
	{
		if (windowEvent.get_type().is_derived_from(RTTI_OF(nap::WindowMovedEvent)))
		{
			// Get display
			nap::RenderWindow* window = mRenderService->findWindow(windowEvent.mWindow); assert(window != nullptr);
			auto display_index = window->getDisplayIndex(); assert(display_index >= 0);

			// Get cached display
			auto it = mContexts.find(window);
			assert(it != mContexts.end());
			const auto& gui_ctx = *it->second;

			// Check if changed, if so update (push) scale
			if (gui_ctx.mDisplayIndex != display_index)
			{
				// Display Changed!
				nap::Logger::debug("Window '%s': Display changed from index %d to %d", window->mID.c_str(),
					gui_ctx.mDisplayIndex, display_index);
				pushScale(*it->second, *window);
			}
		}
	}


	void IMGuiService::createVulkanResources(nap::RenderWindow& window)
	{
		//////////////////////////////////////////////////////////////////////////
		// Create local vulkan resources
		//////////////////////////////////////////////////////////////////////////

		// Create descriptor set pool for ImGUI to use, capped at 1 set, used by font texture
		createFontDescriptorPool(*mRenderService, mDescriptorPool);

		// Create all required vulkan resources
		createDeviceObjects(*mRenderService, mSampler, mDescriptorSetLayout);

		// Create description set allocator for displaying custom NAP textures in ImGUI
		mAllocator = std::make_unique<DescriptorSetAllocator>(mRenderService->getDevice());

		//////////////////////////////////////////////////////////////////////////
		// Create imgui specific vulkan resources
		//////////////////////////////////////////////////////////////////////////

		// Collect ImGUI vulkan init information
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = mRenderService->getVulkanInstance();
		init_info.PhysicalDevice = mRenderService->getPhysicalDevice();
		init_info.Device = mRenderService->getDevice();
		init_info.QueueFamily = mRenderService->getQueueIndex();
		init_info.Queue = mRenderService->getQueue();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = mDescriptorPool;
		init_info.Allocator = VK_NULL_HANDLE;
		init_info.MinImageCount = mRenderService->getMaxFramesInFlight();
		init_info.ImageCount = mRenderService->getMaxFramesInFlight();
		init_info.MSAASamples = window.getSampleCount();
		init_info.CheckVkResultFn = checkVKResult;

		// Create all the vulkan resources, using the window's render pass and init info
		ImGui_ImplVulkan_Init(&init_info, window.getRenderPass());

		// Create the font texture, upload it immediately using a new framebuffer
		VkCommandBuffer font_cmd_buffer = beginSingleTimeCommands(*mRenderService);
		ImGui_ImplVulkan_CreateFontsTexture(font_cmd_buffer);

		// Destroy command buffer and font related uploaded objects
		endSingleTimeCommands(*mRenderService, font_cmd_buffer);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}


	void IMGuiService::newFrame(RenderWindow& window, GUIContext& context, double deltaTime)
	{
		// Switch context
		ImGui::SetCurrentContext(context.mContext);
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = deltaTime;

		// We manage scaling of the GUI manually, removing the need to scale the buffer when high DPI is enabled
		io.DisplaySize = { (float)window.getBufferSize().x, (float)window.getBufferSize().y };
		io.DisplayFramebufferScale = { 1.0f, 1.0f };

		// Set mouse coordinates
		io.MousePos =
		{
			static_cast<float>(context.mMousePosition.x),
			static_cast<float>(window.getHeight() - 1 - context.mMousePosition.y)
		};

		// Compensate for pixel density
		auto pixel_density = window.getPixelDensity();
		io.MousePos.x *= pixel_density;
		io.MousePos.y *= pixel_density;

		// Tell the system which mouse buttons are pressed
		for (auto i = 0; i < context.mMousePressed.size(); i++)
		{
			io.MouseDown[i] = context.mMousePressed[i];

			// Check if the mouse button has been released this frame. Take into consideration current state if press is from a mouse.
			// This is required because the user can release the button outside of SDL window bounds, in which case no release event is generated.
			bool released = context.mMouseRelease[i];
			if (!released && io.MouseDown[i] && context.mPointerID[i] == gui::pointerMouseID)
				released = (SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(i + 1)) == 0;

			// If the mouse button was released this frame -> disable the press for next frame.
			// This ensures that buttons that are pressed and released within the same frame are always registered.
			if (released)
			{
				context.mMousePressed[i] = false;
				context.mMouseRelease[i] = false;
			}
		}

		// Tell the system which keys are pressed, we copy block of memory because iterating over every element is wasteful
		static constexpr size_t keyArraySize = GUIContext::keyCount * sizeof(bool);
		memcpy(io.KeysDown, context.mKeyPressed.data(), keyArraySize);
		for (const auto& key : context.mKeyRelease)
		{
			// If a key was released this frame -> disable the key press for next frame
			// This ensures that keys that are pressed and released within the same frame are always registered
			assert(key < context.mKeyPressed.size());
			context.mKeyPressed[key] = false;
		}
		context.mKeyRelease.clear();

		// Update key modifiers
		io.KeyCtrl = context.mModPressed[modControl];
		io.KeyAlt = context.mModPressed[modAlt];
		io.KeyShift = context.mModPressed[modShift];

		for (auto i = 0; i < context.mModRelease.size(); i++)
		{
			// If a modifier was released -> disable the mod for the next frame
			// This ensures that mod keys that are pressed and released within the same frame are always registered
			if (context.mModRelease[i])
			{
				context.mModPressed[i] = false;
				context.mModRelease[i] = false;
			}
		}

		// Update mouse wheel state
		io.MouseWheel = context.mMouseWheel;

		// Reset mouse wheel
		context.mMouseWheel = 0.0f;

		// Begin new frame
		ImGui::NewFrame();
	}


	void IMGuiService::pushScale(GUIContext& context, const nap::RenderWindow& window)
	{
		// Store display index
		context.mDisplayIndex = window.getDisplayIndex();

		// Compute overall Gui and font scaling factor
		// Overall font scaling factor is always <= 1.0, because the font is created based on the display with the highest DPI value
		float gscale = mGuiScale * window.getDisplayScale();
		float fscale = window.getDisplayScale() / mReferenceScale;

		// Push scaling for window and font based on new display
		// We must push the original style first before we can scale
		context.activate();
		ImGui::GetStyle() = *context.mStyle;
		ImGui::GetStyle().ScaleAllSizes(gscale);
		ImGui::GetIO().FontGlobalScale = fscale;
		context.deactivate();

		// Store scale, ensures custom widgets can scale accordingly
		context.mScale = gscale;
	}


	void IMGuiService::handleKeyEvent(const KeyEvent& keyEvent, GUIContext& context)
	{
		if (keyEvent.mKey == EKeyCode::KEY_UNKNOWN)
			return;

		int key_idx = static_cast<int>(keyEvent.mKey); assert(key_idx < 512);
		int mod_idx = getModKeyIndex(keyEvent.mKey);

		if (keyEvent.get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			context.mKeyPressed[key_idx] = true;
			if (mod_idx >= 0)
			{
				assert(mod_idx < context.mModPressed.size());
				context.mModPressed[mod_idx] = true;
			}
		}

		else if (keyEvent.get_type().is_derived_from(RTTI_OF(nap::KeyReleaseEvent)))
		{
			context.mKeyRelease.emplace_back(key_idx);
			if (mod_idx >= 0)
			{
				assert(mod_idx < context.mModRelease.size());
				context.mModRelease[mod_idx] = true;
			}
		}
	}


	static constexpr int getPointerID(PointerEvent::ESource source)
	{
		return source == PointerEvent::ESource::Mouse ? gui::pointerMouseID :
			gui::pointerTouchID;
	}


	void IMGuiService::handlePointerEvent(const PointerEvent& pointerEvent, GUIContext& context)
	{

		// Handle Press
		if (pointerEvent.get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
		{
			const auto& press_event = static_cast<const nap::PointerPressEvent&>(pointerEvent);
			if (press_event.mButton == PointerClickEvent::EButton::UNKNOWN)
				return;

			int btn_id = static_cast<int>(press_event.mButton);
			context.mMousePosition.x = pointerEvent.mX;
			context.mMousePosition.y = pointerEvent.mY;
			context.mMousePressed[btn_id] = true;
			context.mPointerID[btn_id] = getPointerID(pointerEvent.mSource);
		}

		// Handle Move
		else if (pointerEvent.get_type().is_derived_from(RTTI_OF(nap::PointerMoveEvent)))
		{
			// Don't register move when IDs don't match when pressed
			if (context.mMousePressed[0] && context.mPointerID[0] != getPointerID(pointerEvent.mSource))
				return;

			context.mMousePosition.x = pointerEvent.mX;
			context.mMousePosition.y = pointerEvent.mY;
		}

		// Handle Release
		else if (pointerEvent.get_type().is_derived_from(RTTI_OF(nap::PointerReleaseEvent)))
		{
			const auto& release_event = static_cast<const nap::PointerReleaseEvent&>(pointerEvent);
			if (release_event.mButton == PointerClickEvent::EButton::UNKNOWN)
				return;

			int btn_id = static_cast<int>(release_event.mButton);
			if (context.mPointerID[btn_id] != getPointerID(pointerEvent.mSource))
				return;

			context.mMousePosition.x = pointerEvent.mX;
			context.mMousePosition.y = pointerEvent.mY;
			context.mMouseRelease[btn_id] = true;
			context.mPointerID[btn_id] = gui::pointerInvalidID;
		}
	}


	void IMGuiService::handleTouchEvent(const TouchEvent& touchEvent, GUIContext& context)
	{
		/*
		 * Updates GUI mouse information based on touch input
		 * This function should only be called when touch input is decoupled from the mouse and a window is underneath the touch event.
		 * The GUIAppEventHandler forwards touch events to the GUI if 'setTouchGenerateMouseEvents' is set to false.
		 */

		// Register press, touch is now active ID
		assert(touchEvent.hasWindow());
		if (touchEvent.get_type().is_derived_from(RTTI_OF(nap::TouchPressEvent)))
		{
			context.mMousePressed[0] = true;
			context.mPointerID[0] = touchEvent.mFingerID;
			context.mMousePosition.x = touchEvent.mXCoordinate;
			context.mMousePosition.y = touchEvent.mYCoordinate;
		}

		// Set position if pointer ID is finger ID
		else if (touchEvent.get_type().is_derived_from(RTTI_OF(nap::TouchMoveEvent)))
		{
			if (context.mPointerID[0] != touchEvent.mFingerID)
				return;

			context.mMousePosition.x = touchEvent.mXCoordinate;
			context.mMousePosition.y = touchEvent.mYCoordinate;
		}

		// Release if pointer ID is finger ID
		else if (touchEvent.get_type().is_derived_from(RTTI_OF(nap::TouchReleaseEvent)))
		{
			if (context.mPointerID[0] != touchEvent.mFingerID)
				return;

			context.mMouseRelease[0] = true;
			context.mPointerID[0] = gui::pointerInvalidID;
			context.mMousePosition.x = touchEvent.mXCoordinate;
			context.mMousePosition.y = touchEvent.mYCoordinate;
		}
	}


	void IMGuiService::setPalette(gui::EColorScheme palette)
	{
		// Fetch palette and check if it's different
		const auto* other_palette = gui::getPalette(palette);
		assert(other_palette != nullptr);
		if (other_palette == mColorPalette)
			return;

		// Apply palette to style template
		gui::applyPalette(*other_palette, *mStyle);

		// Apply palette to active contexts
		for (auto& iter : mContexts)
		{
			// Push palette for context
			auto& ctx = *iter.second;
			ctx.activate();
			gui::applyPalette(*other_palette, ImGui::GetStyle());
			ctx.deactivate();
		}

		// Update icons if palette requires it
		utility::ErrorState error;
		if (mColorPalette->mInvertIcon != other_palette->mInvertIcon)
		{
			for (auto& icon : mIcons)
			{
				// Create
				auto new_icon = std::make_unique<Icon>(*this, icon.second->getPath());
				new_icon->mInvert = other_palette->mInvertIcon;
				if (!new_icon->init(error))
				{
					nap::Logger::error("Icon '%s' update failed", icon.first.c_str());
					continue;
				}

				// Replace
				icon.second = std::move(new_icon);
			}
		}
		mColorPalette = other_palette;
	}


	IMGuiService::GUIContext::GUIContext(ImGuiContext* context, ImGuiStyle* style) :
		mContext(context), mStyle(style)
	{ }



	IMGuiService::GUIContext::~GUIContext()
	{
		ImGui::DestroyContext(mContext);
		mStyle = nullptr;
	}


	void IMGuiService::GUIContext::activate()
	{
		mPreviousContext = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(mContext);
	}


	void IMGuiService::GUIContext::deactivate()
	{
		ImGui::SetCurrentContext(mPreviousContext);
	}
}
