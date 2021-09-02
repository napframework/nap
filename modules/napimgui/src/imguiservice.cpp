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

RTTI_BEGIN_CLASS(nap::IMGuiServiceConfiguration)
	RTTI_PROPERTY("FontSize",			&nap::IMGuiServiceConfiguration::mFontSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY_FILELINK("FontFile",	&nap::IMGuiServiceConfiguration::mFontFile,			nap::rtti::EPropertyMetaData::Default, nap::rtti::EPropertyFileType::Font)
	RTTI_PROPERTY("HighlightColor",		&nap::IMGuiServiceConfiguration::mHighlightColor,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BackgroundColor",	&nap::IMGuiServiceConfiguration::mBackgroundColor,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DarkColor",			&nap::IMGuiServiceConfiguration::mDarkColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor1",		&nap::IMGuiServiceConfiguration::mFront1Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor2",		&nap::IMGuiServiceConfiguration::mFront2Color,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrontColor3",		&nap::IMGuiServiceConfiguration::mFront3Color,		nap::rtti::EPropertyMetaData::Default)
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


	static void applyStyle(const IMGuiServiceConfiguration& config)
	{
		// Get ImGUI colors
		ImVec4 IMGUI_NAPDARK(config.mDarkColor, 1.0f);
		ImVec4 IMGUI_NAPBACK(config.mBackgroundColor, 0.94f);
		ImVec4 IMGUI_NAPMODA(config.mDarkColor, 0.85f);
		ImVec4 IMGUI_NAPFRO1(config.mFront1Color, 1.0f);
		ImVec4 IMGUI_NAPFRO2(config.mFront2Color, 1.0f);
		ImVec4 IMGUI_NAPFRO3(config.mFront3Color, 1.0f);
		ImVec4 IMGUI_NAPHIGH(config.mHighlightColor, 1.0f);

		// Apply style
		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowPadding = ImVec2(15, 15);
		style.WindowRounding = 3.0f;
		style.FramePadding = ImVec2(5, 5);
		style.FrameRounding = 2.0f;
		style.ItemSpacing = ImVec2(12, 6);
		style.ItemInnerSpacing = ImVec2(8, 6);
		style.IndentSpacing = 25.0f;
		style.ScrollbarSize = 15.0f;
		style.ScrollbarRounding = 7.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 1.0f;
		style.WindowBorderSize = 0.0f;

		style.Colors[ImGuiCol_Text] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_TextDisabled] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_WindowBg] = IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ChildBg] = IMGUI_NAPBACK;
		style.Colors[ImGuiCol_PopupBg] = IMGUI_NAPBACK;
		style.Colors[ImGuiCol_Border] = IMGUI_NAPDARK;
		style.Colors[ImGuiCol_BorderShadow] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_FrameBg] = IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgHovered] = IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgActive] = IMGUI_NAPDARK;
		style.Colors[ImGuiCol_TitleBg] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBgCollapsed] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TitleBgActive] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_MenuBarBg] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ScrollbarBg] = IMGUI_NAPDARK;
		style.Colors[ImGuiCol_ScrollbarGrab] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_CheckMark] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SliderGrab] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SliderGrabActive] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Button] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ButtonHovered] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ButtonActive] = IMGUI_NAPDARK;
		style.Colors[ImGuiCol_Header] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_Separator] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SeparatorHovered] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SeparatorActive] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_HeaderHovered] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_HeaderActive] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ResizeGrip] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ResizeGripHovered] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ResizeGripActive] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Tab] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TabHovered] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TabActive] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TabUnfocused] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TabUnfocusedActive] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_PlotLines] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_PlotLinesHovered] = IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_PlotHistogram] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_PlotHistogramHovered] = IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_TextSelectedBg] = IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ModalWindowDimBg] = IMGUI_NAPMODA;
		style.Colors[ImGuiCol_Separator] = IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SeparatorHovered] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SeparatorActive] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavHighlight] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavWindowingHighlight] = IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavWindowingDimBg] = IMGUI_NAPMODA;
		style.Colors[ImGuiCol_DragDropTarget] = IMGUI_NAPHIGH;
	}


	static ImGuiContext* createContext(const IMGuiServiceConfiguration& configuration,  ImFontAtlas& fontAtlas, float dpiScale)
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

		// Push default style
		applyStyle(configuration);
		ImGui::GetStyle().ScaleAllSizes(dpiScale);

		// Reset context
		ImGui::SetCurrentContext(cur_context);
		return new_context;
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
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
			it->second->mContext,
			mRenderService->getCurrentCommandBuffer(),
			current_window->getRenderPass(),
			current_window->getSampleCount());
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
			context->second->mMouseWheel = wheel_event.mY > 0 ? 1 : -1;
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


	bool IMGuiService::init(utility::ErrorState& error)
	{
		// Get our renderer
		mRenderService = getCore().getService<nap::RenderService>();
		assert(mRenderService != nullptr);

		// Register to window added / removed signals
		mRenderService->windowAdded.connect(mWindowAddedSlot);
		mRenderService->windowRemoved.connect(mWindowRemovedSlot);

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
			int display_idx = SDL::getDisplayIndex(window.getNativeWindow());
			if (display_idx < 0)
			{
				nap::Logger::error("Unable to get display index for window: %s", window.mID.c_str());
				display_idx = 0;
			}

			// Create atlas, scale based on dpi of main monitor
			mFontAtlas = createFontAtlas(display_idx, mDPIScale);

			// Create context and apply style
			new_context = createContext(*getConfiguration<IMGuiServiceConfiguration>(), *mFontAtlas, mDPIScale);

			// Create all vulkan required resources
			createVulkanResources(window);
		}
		else
		{
			// New context for window
			new_context = createContext(*getConfiguration<IMGuiServiceConfiguration>(), *mFontAtlas, mDPIScale);
		}

		// Add context
		mContexts.emplace(std::make_pair(&window, std::make_unique<GUIContext>(new_context)));

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
			// Get window
			nap::RenderWindow* window = mRenderService->findWindow(windowEvent.mWindow);
			assert(window != nullptr);

			// Get display
			int idx = SDL::getDisplayIndex(window->getNativeWindow());
			assert(idx >= 0);

			// Check if it changed
			nap::Logger::info("Display Index: %d", idx);
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

		// Setup display size (every frame to accommodate for window resizing)
		ImGuiIO& io = ImGui::GetIO();
		int display_w, display_h;
		SDL_GL_GetDrawableSize(window.getNativeWindow(), &display_w, &display_h);
		io.DisplaySize = { (float)display_w, (float)display_h };
		io.DisplayFramebufferScale = {1.0f, 1.0f};
		io.DeltaTime = deltaTime;

		// Setup inputs. We already got mouse wheel, keyboard keys & characters from SDL_PollEvent()
                // Scale SDL mouse position based on width & height in pixels, since we draw everything in
		int mx, my;
		Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
                if(SDL_GetWindowFlags(window.getNativeWindow()) & SDL_WINDOW_MOUSE_FOCUS)
                {
                  int w, h;
                  SDL_GetWindowSize(window.getNativeWindow(), &w, &h);
                  io.MousePos =
                  {
                      static_cast<float>(mx) * (static_cast<float>(display_w) / static_cast<float>(w)),
                      static_cast<float>(my) * (static_cast<float>(display_h) / static_cast<float>(h))
                  };
                }
                else
                {
                  io.MousePos = { -FLT_MAX, -FLT_MAX };
                }

		// Update mouse down state
		io.MouseDown[0] = context.mMousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[1] = context.mMousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2] = context.mMousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		context.mMousePressed[0] = context.mMousePressed[1] = context.mMousePressed[2] = false;

		// Update mouse wheel state
		io.MouseWheel = context.mMouseWheel;
		context.mMouseWheel = 0.0f;

		// Begin new frame
		ImGui::NewFrame();
	}


	std::unique_ptr<ImFontAtlas> IMGuiService::createFontAtlas(int displayIndex, float& outDPIScale)
	{
		// Create font atlas
		std::unique_ptr<ImFontAtlas> new_atlas = std::make_unique<ImFontAtlas>();
		ImFontConfig font_config;
		font_config.OversampleH = 3;
		font_config.OversampleV = 1;

		// Compute dpi scale if not headless and dpi scaling is enabled
		outDPIScale = 1.0f;
		if (!mRenderService->isHeadless() && mRenderService->getHighDPIEnabled())
		{
			int num_displays = SDL::getDisplayCount();
            nap::Logger::info("Found %d displays", num_displays);
            for(int i=0; i<num_displays; i++)
            {
                float ddpi = 0.0f;
                float hdpi = 0.0f;
                float vdpi = 0.0f;
                nap::Logger::info("Display %d: name: %s", i, SDL_GetDisplayName(i));
                SDL::getDisplayDPI(i, &ddpi, &hdpi, &vdpi);
                nap::Logger::info("Display %d: ddpi: %.2f, hdpi: %.2f, vdpi: %.2f", i, ddpi, hdpi, vdpi);
            }

			if (num_displays - 1 < displayIndex)
			{
				nap::Logger::error("Unable to calculate DPI scaling factor, display index out of bounds");
			}
			else
			{
				float dpi = 0.0f;
				if (SDL::getDisplayDPI(displayIndex, nullptr, &dpi, nullptr) < 0)
				{
					nap::Logger::error(SDL::getSDLError());
				}
				else
				{
					outDPIScale = dpi / 96.0f;
				}
			}
		}

		// Add font, scale based on main dpi (TODO: Make Monitor Aware)
		auto config = getConfiguration<IMGuiServiceConfiguration>();
		float font_size = math::floor(config->mFontSize * outDPIScale);
		if (!config->mFontFile.empty())
		{
			new_atlas->AddFontFromFileTTF(config->mFontFile.c_str(), font_size, &font_config);
		}
		else
		{
			new_atlas->AddFontFromMemoryCompressedTTF(nunitoSansSemiBoldData, nunitoSansSemiBoldSize, font_size, &font_config);
		}
		return new_atlas;
	}


	IMGuiService::GUIContext::~GUIContext()
	{
		ImGui::DestroyContext(mContext);
	}
}
