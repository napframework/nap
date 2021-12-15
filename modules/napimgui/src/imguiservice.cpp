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
#include <SDL_syswm.h>
#include <SDL_mouse.h>
#include <SDL_keyboard.h>
#include <nap/logger.h>
#include <materialcommon.h>
#include <descriptorsetallocator.h>
#include <sdlhelpers.h>

RTTI_BEGIN_STRUCT(nap::IMGuiColorPalette)
	RTTI_PROPERTY("HighlightColor",		&nap::IMGuiColorPalette::mHighlightColor,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BackgroundColor",	&nap::IMGuiColorPalette::mBackgroundColor,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DarkColor",			&nap::IMGuiColorPalette::mDarkColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor1",		&nap::IMGuiColorPalette::mFront1Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor2",		&nap::IMGuiColorPalette::mFront2Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor3",		&nap::IMGuiColorPalette::mFront3Color,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::IMGuiServiceConfiguration)
	RTTI_PROPERTY("FontSize",			&nap::IMGuiServiceConfiguration::mFontSize,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GlobalScale",		&nap::IMGuiServiceConfiguration::mScale,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY_FILELINK("FontFile",	&nap::IMGuiServiceConfiguration::mFontFile,		nap::rtti::EPropertyMetaData::Default, nap::rtti::EPropertyFileType::Font)
	RTTI_PROPERTY("Colors",				&nap::IMGuiServiceConfiguration::mColors,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMGuiService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

// Static data associated with IMGUI
static VkDescriptorPool			gDescriptorPool = VK_NULL_HANDLE;
static VkDescriptorSetLayout    gDescriptorSetLayout = VK_NULL_HANDLE;
static VkSampler                gSampler = VK_NULL_HANDLE;

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Icons
	//////////////////////////////////////////////////////////////////////////

	static const std::vector<std::string>& getIcons()
	{
		const static std::vector<std::string> map =
		{
			icon::save,		icon::saveAs,	icon::cancel,
			icon::remove,	icon::file,		icon::help,
			icon::settings,	icon::verify,	icon::reload
		};
		return map;
	}


	std::unique_ptr<nap::ImageFromFile> icon::load(const std::string& fileName, bool generateLODs, nap::Core& core, const Module& module, nap::utility::ErrorState& error)
	{
		// Find path to asset
		auto icon_path = module.findAsset(fileName);
		if (!error.check(!icon_path.empty(), "%s: Unable to find icon %s", module.getName().c_str(), fileName.c_str()))
			return false;

		// Create icon
		auto new_icon = std::make_unique<ImageFromFile>(core, icon_path);
		new_icon->mGenerateLods = generateLODs;
		if (!new_icon->init(error))
			new_icon.reset(nullptr);

		return new_icon;
	}


	std::unique_ptr<nap::ImageFromFile> icon::load(const std::string& fileName, bool generateLODs, nap::Service& service, nap::utility::ErrorState& error)
	{
		return icon::load(fileName, generateLODs, service.getCore(), service.getModule(), error);
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


	static void createDeviceObjects(RenderService& renderService)
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
		err = vkCreateSampler(renderService.getDevice(), &info, nullptr, &gSampler);
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
		err = vkCreateDescriptorSetLayout(renderService.getDevice(), &set_info, nullptr, &gDescriptorSetLayout);
		checkVKResult(err);
	}


	static void destroyDeviceObjects(RenderService& renderService)
	{
		if (gDescriptorSetLayout)	{ vkDestroyDescriptorSetLayout(renderService.getDevice(), gDescriptorSetLayout, nullptr); }
		if (gSampler)				{ vkDestroySampler(renderService.getDevice(), gSampler, nullptr); }
	}


	// Create descriptor pool for imgui vulkan implementation, allows creation / allocation right resources on that side
	static void createFontDescriptorPool(RenderService& renderService)
	{
		VkDescriptorPoolSize pool_size = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)(1) };

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &pool_size;
		poolInfo.maxSets = 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VkResult result = vkCreateDescriptorPool(renderService.getDevice(), &poolInfo, nullptr, &gDescriptorPool);
		assert(result == VK_SUCCESS);
	}


	static void destroyFontDescriptorPool(RenderService& renderService)
	{
		vkDestroyDescriptorPool(renderService.getDevice(), gDescriptorPool, nullptr);
	}


	static const char* getClipboardText(void*)
	{
		return SDL_GetClipboardText();
	}


	static void setClipboardText(void*, const char* text)
	{
		SDL_SetClipboardText(text);
	}


	static std::unique_ptr<ImGuiStyle> createStyle(const IMGuiServiceConfiguration& config)
	{
		// Get ImGUI colors
		ImVec4 IMGUI_NAPDARK(config.mColors.mDarkColor, 1.0f);
		ImVec4 IMGUI_NAPBACK(config.mColors.mBackgroundColor, 0.94f);
		ImVec4 IMGUI_NAPMODA(config.mColors.mDarkColor, 0.85f);
		ImVec4 IMGUI_NAPFRO1(config.mColors.mFront1Color, 1.0f);
		ImVec4 IMGUI_NAPFRO2(config.mColors.mFront2Color, 1.0f);
		ImVec4 IMGUI_NAPFRO3(config.mColors.mFront3Color, 1.0f);
		ImVec4 IMGUI_NAPHIGH(config.mColors.mHighlightColor, 1.0f);

		// Create style
		std::unique_ptr<ImGuiStyle> style = std::make_unique<ImGuiStyle>();

		// Apply settings from config
		style->WindowPadding = ImVec2(10, 10);
		style->WindowRounding = 0.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 0.0f;
		style->ItemSpacing = ImVec2(12, 6);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 12.0f;
		style->ScrollbarRounding = 7.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 0.0f;
		style->WindowBorderSize = 0.0f;
		style->PopupRounding = 0.0f;
		style->ChildRounding = 0.0f;
        style->WindowTitleAlign = { 0.5f, 0.5f };
		style->PopupBorderSize = 0.0f;

		style->Colors[ImGuiCol_Text] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_TextDisabled] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_WindowBg] = IMGUI_NAPBACK;
		style->Colors[ImGuiCol_ChildBg] = IMGUI_NAPBACK;
		style->Colors[ImGuiCol_PopupBg] = IMGUI_NAPBACK;
		style->Colors[ImGuiCol_Border] = IMGUI_NAPDARK;
		style->Colors[ImGuiCol_BorderShadow] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_FrameBg] = IMGUI_NAPDARK;
		style->Colors[ImGuiCol_FrameBgHovered] = IMGUI_NAPDARK;
		style->Colors[ImGuiCol_FrameBgActive] = IMGUI_NAPDARK;
		style->Colors[ImGuiCol_TitleBg] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_TitleBgCollapsed] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_TitleBgActive] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_MenuBarBg] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_ScrollbarBg] = IMGUI_NAPDARK;
		style->Colors[ImGuiCol_ScrollbarGrab] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_ScrollbarGrabActive] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_CheckMark] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_SliderGrab] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_SliderGrabActive] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_Button] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_ButtonHovered] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_ButtonActive] = IMGUI_NAPDARK;
		style->Colors[ImGuiCol_Header] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_Separator] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_SeparatorHovered] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_SeparatorActive] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_HeaderHovered] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_HeaderActive] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_ResizeGrip] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_ResizeGripHovered] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_ResizeGripActive] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_Tab] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_TabHovered] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_TabActive] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_TabUnfocused] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_TabUnfocusedActive] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_PlotLines] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_PlotLinesHovered] = IMGUI_NAPHIGH;
		style->Colors[ImGuiCol_PlotHistogram] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_PlotHistogramHovered] = IMGUI_NAPHIGH;
		style->Colors[ImGuiCol_TextSelectedBg] = IMGUI_NAPFRO1;
		style->Colors[ImGuiCol_ModalWindowDimBg] = IMGUI_NAPMODA;
		style->Colors[ImGuiCol_Separator] = IMGUI_NAPFRO2;
		style->Colors[ImGuiCol_SeparatorHovered] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_SeparatorActive] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_NavHighlight] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_NavWindowingHighlight] = IMGUI_NAPFRO3;
		style->Colors[ImGuiCol_NavWindowingDimBg] = IMGUI_NAPMODA;
		style->Colors[ImGuiCol_DragDropTarget] = IMGUI_NAPHIGH;

		return style;
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
		io.KeyMap[ImGuiKey_Enter] = (int)EKeyCode::KEY_KP_ENTER;
		io.KeyMap[ImGuiKey_Escape] = (int)EKeyCode::KEY_ESCAPE;
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


	static std::unique_ptr<ImFontAtlas> createFontAtlas(float fontSize, const char* fontFile = nullptr)
	{
		// Create font atlas
		std::unique_ptr<ImFontAtlas> new_atlas = std::make_unique<ImFontAtlas>();
		ImFontConfig font_config;
		font_config.OversampleH = 3;
		font_config.OversampleV = 1;

		// Add font, scale based on main dpi (TODO: Make Monitor Aware)
		float font_size = math::floor(fontSize);
		if (fontFile != nullptr)
		{
			new_atlas->AddFontFromFileTTF(fontFile, font_size, &font_config);
		}
		else
		{
			new_atlas->AddFontFromMemoryCompressedTTF(nunitoSansSemiBoldData, nunitoSansSemiBoldSize, font_size, &font_config);
		}
		return new_atlas;
	}


	static void setGuiWindow(SDL_Window* window)
	{
#ifdef _WIN32
		ImGuiIO& io = ImGui::GetIO();
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window, &wmInfo);
		io.ImeWindowHandle = wmInfo.info.win.window;
#else
		(void)window;
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	// Service Methods
	//////////////////////////////////////////////////////////////////////////

	IMGuiService::IMGuiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


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


	ImGuiContext* IMGuiService::processInputEvent(InputEvent& event)
	{
		// Check if it's a window input event
		if (!event.get_type().is_derived_from(RTTI_OF(nap::WindowInputEvent)))
			return nullptr;

		// Find window associated with event
		WindowInputEvent& input_event = static_cast<WindowInputEvent&>(event);
		nap::RenderWindow* window = mRenderService->findWindow(input_event.mWindow);
		assert(window != nullptr);

		// Get context for window
		auto context = mContexts.find(window);
		assert(context != mContexts.end());

		// Select contect and get input / output information
		ImGui::SetCurrentContext(context->second->mContext);
		ImGuiIO& io = ImGui::GetIO();

		// Key event
		if (event.get_type().is_derived_from(RTTI_OF(nap::KeyEvent)))
		{
			bool pressed = event.get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent));
			KeyEvent& key_event = static_cast<KeyEvent&>(event);
			io.KeysDown[(int)(key_event.mKey)] = pressed;
			io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
			io.KeyCtrl	= ((SDL_GetModState() & KMOD_CTRL)	!= 0);
			io.KeyAlt	= ((SDL_GetModState() & KMOD_ALT)	!= 0);
			io.KeySuper	= ((SDL_GetModState() & KMOD_GUI)	!= 0);
		}

		// Text input event
		else if (event.get_type().is_derived_from(RTTI_OF(nap::TextInputEvent)))
		{
			nap::TextInputEvent& press_event = static_cast<nap::TextInputEvent&>(event);
			io.AddInputCharactersUTF8(press_event.mText.c_str());
		}

		// Mouse wheel event
		else if (event.get_type().is_derived_from(RTTI_OF(nap::MouseWheelEvent)))
		{
			nap::MouseWheelEvent& wheel_event = static_cast<nap::MouseWheelEvent&>(event);
#ifdef __APPLE__
			int delta = io.KeyShift ? wheel_event.mX * -1 : wheel_event.mY;
#else
			int delta = wheel_event.mY;
#endif
			context->second->mMouseWheel = delta > 0 ? 1.0f : -1.0f;
		}

		// Pointer press event
		else if (event.get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
		{
			nap::PointerPressEvent& press_event = static_cast<nap::PointerPressEvent&>(event);
			if(press_event.mButton != EMouseButton::UNKNOWN)
				context->second->mMousePressed[static_cast<int>(press_event.mButton)] = true;
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


	ImTextureID IMGuiService::getTextureHandle(nap::Texture2D& texture)
	{
		// Check if the texture has been requested before
		auto it = mDescriptors.find(&texture);
		if (it != mDescriptors.end())
			return (ImTextureID)(it->second);

		// Allocate new description set
		VkDescriptorSet descriptor_set = mAllocator->allocate(gDescriptorSetLayout, 0, 1);

		// Update description set
		VkDescriptorImageInfo desc_image[1] = {};
		desc_image[0].sampler = gSampler;
		desc_image[0].imageView = texture.getImageView();
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


	const nap::IMGuiColorPalette& IMGuiService::getColors() const
	{
		assert(mConfiguration != nullptr);
		return mConfiguration->mColors;
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
		mGuiScale = math::max<float>(mConfiguration->mScale, 0.05f);

		// Load all the default icons
		bool icons_loaded = true;
		const auto& default_icons = getIcons();
		for (const auto& icon_name : default_icons)
		{
			auto new_icon = icon::load(icon_name, true, *this, error);
			if (new_icon == nullptr)
			{
				mIcons.clear();
				return false;
			}
			mIcons.emplace_back(std::move(new_icon));
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
			destroyDeviceObjects(*mRenderService);
			destroyFontDescriptorPool(*mRenderService);
			mAllocator.reset(nullptr);
		}

		// Destroy imgui contexts
		mContexts.clear();

		// Destroy icons
		mIcons.clear();
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
			// Calculate max dpi scale if high dpi rendering is enabled
			if (mRenderService->getHighDPIEnabled())
			{
				for (const auto& display : mRenderService->getDisplays())
				{
					float dpi_scale = math::max<float>(display.getHorizontalDPI(), gui::dpi) / gui::dpi;
					//nap::Logger::info("Display: %d, DPI Scale: %.2f", display.getIndex(), dpi_scale);
					mDPIScale = dpi_scale > mDPIScale ? dpi_scale : mDPIScale;
				}
			}

			// Create atlas, scale based on dpi of main monitor
			const char* font_file = mConfiguration->mFontFile.empty() ? nullptr : mConfiguration->mFontFile.c_str();
			float font_size = mConfiguration->mFontSize * mDPIScale * mGuiScale;
			mFontAtlas = createFontAtlas(font_size, font_file);

			// Create style
			mStyle = createStyle(*getConfiguration<IMGuiServiceConfiguration>());

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
		const auto* display = mRenderService->findDisplay(window);
		assert(display != nullptr);
		pushScale(*it.first->second, *display);

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
			nap::RenderWindow* window = mRenderService->findWindow(windowEvent.mWindow);
			assert(window != nullptr);
			const Display* display = mRenderService->findDisplay(*window);
			assert(display != nullptr);

			// Get cached display
			auto it = mContexts.find(window);
			assert(it != mContexts.end());
			assert(it->second->mDisplay != nullptr);
			const auto& cached_display = *it->second->mDisplay;

			// Check if changed, if so update (push) scale
			if (cached_display != *display)
			{
				// Display Changed!
				//nap::Logger::info("Display changed from: %d to %d", cached_display.getIndex(), display->getIndex());
				pushScale(*it->second, *display);
			}
		}
	}


	void IMGuiService::createVulkanResources(nap::RenderWindow& window)
	{
		//////////////////////////////////////////////////////////////////////////
		// Create local vulkan resources
		//////////////////////////////////////////////////////////////////////////

		// Create descriptor set pool for ImGUI to use, capped at 1 set, used by font texture
		createFontDescriptorPool(*mRenderService);

		// Create all required vulkan resources
		createDeviceObjects(*mRenderService);

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
		init_info.DescriptorPool = gDescriptorPool;
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

		// Current mouse settings
		int mx, my;
		Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
		bool mouse_focus = SDL_GetWindowFlags(window.getNativeWindow()) & SDL_WINDOW_MOUSE_FOCUS;

		// Mouse coordinates are scaled accordingly when high dpi rendering is enabled
		if (mouse_focus)
		{
			io.MousePos = ImVec2(static_cast<float>(mx), static_cast<float>(my));
			if (mRenderService->getHighDPIEnabled())
			{
				io.MousePos.x *= static_cast<float>(window.getBufferSize().x) / static_cast<float>(window.getWidth());
				io.MousePos.y *= static_cast<float>(window.getBufferSize().y) / static_cast<float>(window.getHeight());
			}
		}
		else
		{
			io.MousePos = { -math::max<float>(), -math::max<float>() };
		}

		// Update mouse down state
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[0] = context.mMousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT))   != 0;		
		io.MouseDown[1] = context.mMousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT))  != 0;
		io.MouseDown[2] = context.mMousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		context.mMousePressed[0] = context.mMousePressed[1] = context.mMousePressed[2] = false;

		// Update mouse wheel state
		io.MouseWheel = context.mMouseWheel;
		context.mMouseWheel = 0.0f;

		// Begin new frame
		ImGui::NewFrame();
	}


	void IMGuiService::pushScale(GUIContext& context, const Display& display)
	{
		// Store display
		context.mDisplay = &display;

		// Don't scale if high dpi rendering is disabled
		if (mRenderService->getHighDPIEnabled())
		{
			// Compute overall Gui and font scaling factor
			// Overall font scaling factor is always <= 1.0, because the font is created based on the display with the highest DPI value
			float gscale = mGuiScale * (math::max<float>(display.getHorizontalDPI(), gui::dpi) / gui::dpi);
			float fscale = math::max<float>(display.getHorizontalDPI(), gui::dpi) / (mDPIScale * gui::dpi);
			//nap::Logger::info("font scale: %.2f", fscale);

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
		else
		{
			context.activate();
			ImGui::GetStyle() = *context.mStyle;
			ImGui::GetStyle().ScaleAllSizes(mGuiScale);
			context.deactivate();

			// Store scale, ensures custom widgets can scale accordingly
			context.mScale = mGuiScale;
		}
	}


	IMGuiService::GUIContext::GUIContext(ImGuiContext* context, ImGuiStyle* style) :
		mContext(context), mStyle(style)
	{ }



	IMGuiService::GUIContext::~GUIContext()
	{
		ImGui::DestroyContext(mContext);
		mStyle = nullptr;
		mDisplay = nullptr;
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
