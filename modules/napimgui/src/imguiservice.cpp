// Local Includes
#include "imguiservice.h"
#include "imguifont.h"
#include "imgui/imgui.h"
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

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMGuiService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

#define IMGUI_VK_QUEUED_FRAMES 2

// Static data associated with IMGUI: TODO: Use own render classes and remove global state!
static bool					gMousePressed[3] = { false, false, false };
static float				gMouseWheel = 0.0f;
static VkDescriptorPool		gDescriptorPool = VK_NULL_HANDLE;

namespace nap
{
	static void checkVKResult(VkResult err)
	{
		if (err == 0) return;
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
		vkAllocateCommandBuffers(renderService.getDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}


	static void endSingleTimeCommands(RenderService& renderService, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(renderService.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(renderService.getGraphicsQueue());
		vkFreeCommandBuffers(renderService.getDevice(), renderService.getCommandPool(), 1, &commandBuffer);
	}


	static const char* getClipboardText(void*)
	{
		return SDL_GetClipboardText();
	}


	static void setClipboardText(void*, const char* text)
	{
		SDL_SetClipboardText(text);
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


	static void newFrame(GLWindow& window)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		SDL_GetWindowSize(window.getNativeWindow(), &w, &h);
		SDL_GL_GetDrawableSize(window.getNativeWindow(), &display_w, &display_h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

		// Setup inputs
		// (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
		int mx, my;
		Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
		
		// Mouse position, in pixels, set to -1,-1 if no mouse / on another screen, etc.
		io.MousePos = SDL_GetWindowFlags(window.getNativeWindow()) & SDL_WINDOW_MOUSE_FOCUS ?
			ImVec2((float)mx, (float)my) : io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

		io.MouseDown[0] = gMousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT))		!= 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[1] = gMousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT))	!= 0;
		io.MouseDown[2] = gMousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE))	!= 0;
		gMousePressed[0] = gMousePressed[1] = gMousePressed[2] = false;

		io.MouseWheel = gMouseWheel;
		gMouseWheel = 0.0f;

		// Start the frame. 
		// This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
		ImGui::NewFrame();
	}


	static void applyStyle()
	{
		// Get colors
		static const RGBColorFloat NAPDARK = RGBColor8(0x11, 0x14, 0x26).convert<RGBColorFloat>();
		static const RGBColorFloat NAPBACK = RGBColor8(0x2D, 0x2E, 0x42).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO1 = RGBColor8(0x52, 0x54, 0x6A).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO2 = RGBColor8(0x5D, 0x5E, 0x73).convert<RGBColorFloat>();
		static const RGBColorFloat NAPFRO3 = RGBColor8(0x8B, 0x8C, 0xA0).convert<RGBColorFloat>();
		static const RGBColorFloat NAPHIGH = RGBColor8(0xC8, 0x69, 0x69).convert<RGBColorFloat>();

		const ImVec4 IMGUI_NAPDARK(NAPDARK.getRed(), NAPDARK.getGreen(), NAPDARK.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPBACK(NAPBACK.getRed(), NAPBACK.getGreen(), NAPBACK.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPMODAL(NAPBACK.getRed(), NAPBACK.getGreen(), NAPBACK.getBlue(), 0.5f);
		const ImVec4 IMGUI_NAPFRO1(NAPFRO1.getRed(), NAPFRO1.getGreen(), NAPFRO1.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPFRO2(NAPFRO2.getRed(), NAPFRO2.getGreen(), NAPFRO2.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPFRO3(NAPFRO3.getRed(), NAPFRO3.getGreen(), NAPFRO3.getBlue(), 1.0f);
		const ImVec4 IMGUI_NAPHIGH(NAPHIGH.getRed(), NAPHIGH.getGreen(), NAPHIGH.getBlue(), 1.0f);

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

		style.Colors[ImGuiCol_Text]						= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_TextDisabled]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_WindowBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ChildBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_PopupBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_Border]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_BorderShadow]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_FrameBg]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgHovered]			= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgActive]			= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_TitleBg]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBgCollapsed]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TitleBgActive]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_MenuBarBg]				= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ScrollbarBg]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_ScrollbarGrab]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ScrollbarGrabHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ScrollbarGrabActive]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_CheckMark]				= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SliderGrab]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SliderGrabActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Button]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ButtonHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ButtonActive]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_Header]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_Separator]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SeparatorHovered]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SeparatorActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_HeaderHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_HeaderActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ResizeGrip]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ResizeGripHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ResizeGripActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Tab]						= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TabHovered]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TabActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_TabUnfocused]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TabUnfocusedActive]		= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_PlotLines]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_PlotLinesHovered]			= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_PlotHistogram]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_PlotHistogramHovered]		= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_TextSelectedBg]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ModalWindowDarkening]		= IMGUI_NAPMODAL;
		style.Colors[ImGuiCol_Separator]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SeparatorHovered]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SeparatorActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavHighlight]				= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavWindowingHighlight]	= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_NavWindowingDimBg]		= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ModalWindowDimBg]			= IMGUI_NAPFRO1;
	}


	IMGuiService::IMGuiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	void IMGuiService::draw(VkCommandBuffer commandBuffer)
	{
		if (mUserWindow == nullptr)
		{
			nap::Logger::error("No GUI target window specified, call `selectWindow()` on init() of the application");
			return;
		}

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}


	void IMGuiService::selectWindow(nap::ResourcePtr<RenderWindow> window)
	{
		if (mUserWindow != nullptr)
		{
			// TODO: This is not correct, many objects can persist, only the pipeline needs to be recreated
			// TODO: Properly handle resource creation for multiple windows
			ImGui_ImplVulkan_Shutdown();
		}

		// Collect init information
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = mRenderService->getVulkanInstance();
		init_info.PhysicalDevice = mRenderService->getPhysicalDevice();
		init_info.Device = mRenderService->getDevice();
		init_info.QueueFamily = mRenderService->getGraphicsQueueIndex();
		init_info.Queue = mRenderService->getGraphicsQueue();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = gDescriptorPool;
		init_info.Allocator = VK_NULL_HANDLE;
		init_info.MinImageCount = mRenderService->getMaxFramesInFlight();
		init_info.ImageCount = mRenderService->getMaxFramesInFlight();		
		init_info.MSAASamples = window->getBackbuffer().getSampleCount();
		init_info.CheckVkResultFn = checkVKResult;

		// Create all the vulkan resources, using the window's render pass and init info
		ImGui_ImplVulkan_Init(&init_info, window->getBackbuffer().getRenderPass());

		// Create the font texture, upload it immediately using a new framebuffer
		VkCommandBuffer font_cmd_buffer = beginSingleTimeCommands(*mRenderService);
		ImGui_ImplVulkan_CreateFontsTexture(font_cmd_buffer);
		
		// Destroy command buffer and font related uploaded objects
		endSingleTimeCommands(*mRenderService, font_cmd_buffer);
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		// Update SDL Window information
		setGuiWindow(window->getWindow()->getNativeWindow());

		// Store handle
		mUserWindow = window;
		mWindowChanged = true;
	}


	void IMGuiService::processInputEvent(InputEvent& event)
	{

		ImGuiIO& io = ImGui::GetIO();
		
		// Key event
		if (event.get_type().is_derived_from(RTTI_OF(nap::KeyEvent)))
		{
			bool pressed = event.get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent));
			KeyEvent& key_event = static_cast<KeyEvent&>(event);
			io.KeysDown[(int)(key_event.mKey)] = pressed;

			// TODO: Keep track of modifier states in NAP so we don't directly interface with SDL2 here
			// Goal is to remove SDL calls from imgui service
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
			gMouseWheel = wheel_event.mY > 0 ? 1 : -1;
		}

		// Pointer Event
		else if (event.get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
		{
			nap::PointerPressEvent& press_event = static_cast<nap::PointerPressEvent&>(event);
			assert(press_event.mButton != EMouseButton::UNKNOWN);
			gMousePressed[static_cast<int>(press_event.mButton)] = true;
		}
	}


	bool IMGuiService::isCapturingKeyboard()
	{
		// Get the interface
		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureKeyboard;
	}


	bool IMGuiService::isCapturingMouse()
	{
		// Get the interface
		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureMouse;
	}


	bool IMGuiService::init(utility::ErrorState& error)
	{
		// Get our renderer
		mRenderService = getCore().getService<nap::RenderService>();
		assert(mRenderService != nullptr);

		// Create descriptor pool
		VkDescriptorPoolSize pool_size = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)(1) };
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &pool_size;
		poolInfo.maxSets = 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VkResult result = vkCreateDescriptorPool(mRenderService->getDevice(), &poolInfo, nullptr, &gDescriptorPool);
		assert(result == VK_SUCCESS);

		// Create ImGUI context
		mContext = ImGui::CreateContext();

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab]			= (int)EKeyCode::KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow]	= (int)EKeyCode::KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow]	= (int)EKeyCode::KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow]		= (int)EKeyCode::KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow]	= (int)EKeyCode::KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp]		= (int)EKeyCode::KEY_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown]	= (int)EKeyCode::KEY_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home]		= (int)EKeyCode::KEY_HOME;
		io.KeyMap[ImGuiKey_End]			= (int)EKeyCode::KEY_END;
		io.KeyMap[ImGuiKey_Delete]		= (int)EKeyCode::KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace]	= (int)EKeyCode::KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter]		= (int)EKeyCode::KEY_KP_ENTER;
		io.KeyMap[ImGuiKey_Escape]		= (int)EKeyCode::KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A]			= (int)EKeyCode::KEY_a;
		io.KeyMap[ImGuiKey_C]			= (int)EKeyCode::KEY_c;
		io.KeyMap[ImGuiKey_V]			= (int)EKeyCode::KEY_v;
		io.KeyMap[ImGuiKey_X]			= (int)EKeyCode::KEY_x;
		io.KeyMap[ImGuiKey_Y]			= (int)EKeyCode::KEY_y;
		io.KeyMap[ImGuiKey_Z]			= (int)EKeyCode::KEY_z;

		// Set callbacks
		io.SetClipboardTextFn = setClipboardText;
		io.GetClipboardTextFn = getClipboardText;
		io.ClipboardUserData = NULL;

		// Push default style
		applyStyle();

		// Add font
		ImFontConfig font_config;
		font_config.OversampleH = 3;
		font_config.OversampleV = 1;
		io.Fonts->AddFontFromMemoryCompressedTTF(nunitoSansSemiBoldData, nunitoSansSemiBoldSize, 17.5f, &font_config);

		return true;
	}


	void IMGuiService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
		dependencies.emplace_back(RTTI_OF(RenderService));
	}


	void IMGuiService::update(double deltaTime)
	{
		if (mUserWindow == nullptr)
		{
			nap::Logger::error("No GUI target window specified, make sure to call `selectWindow()` first");
			return;
		}
		
		// Signal the beginning of a new frame to the vulkan backend
		ImGui_ImplVulkan_NewFrame();

		// Signal the beginning of a new frame to the imgui backend
		newFrame(*mUserWindow->getWindow());
	};


	void IMGuiService::shutdown()
	{
		// Destroy vulkan resources
		ImGui_ImplVulkan_Shutdown();

		// Destroy descriptor pool
		vkDestroyDescriptorPool(mRenderService->getDevice(), gDescriptorPool, nullptr);

		// Destroy imgui context
		ImGui::DestroyContext(mContext);
	}
}
