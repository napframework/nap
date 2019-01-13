// Local Includes
#include "imguiservice.h"
#include "imguifont.h"
#include "imgui/imgui.h"

// External Includes
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <nap/core.h>
#include <color.h>
#include <GL/glew.h>
#include <SDL_clipboard.h>
#include <SDL_syswm.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IMGuiService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

// Static data associated with IMGUI: TODO: Use own render classes and remove global state!
static bool         gMousePressed[3] = { false, false, false };
static float        gMouseWheel = 0.0f;
static GLuint       gFontTexture = 0;
static int          gShaderHandle = 0, gVertHandle = 0, gFragHandle = 0;
static int          gAttribLocationTex = 0, gAttribLocationProjMtx = 0;
static int          gAttribLocationPosition = 0, gAttribLocationUV = 0, gAttribLocationColor = 0;
static unsigned int gVboHandle = 0, gVaoHandle = 0, gElementsHandle = 0;

namespace nap
{

	/**
	 * This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
	 * Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
	 * If text or lines are blurry when integrating ImGui in your engine: in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
	 */
	static void renderDrawLists(ImDrawData* drawData)
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO& io = ImGui::GetIO();
		int fb_width  = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
			return;

		drawData->ScaleClipRects(io.DisplayFramebufferScale);

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		opengl::enableBlending(true);
		opengl::enableFaceCulling(false);
		opengl::enableDepthTest(false);
		opengl::enableScissorTest(true);
		opengl::setPolygonMode(opengl::EPolygonMode::Fill);

		// Setup viewport, orthographic projection matrix
		opengl::setViewport(fb_width, fb_height);
		glm::mat4 ortho_projection = {
			{ 2.0f / io.DisplaySize.x,	0.0f,						0.0f,	0.0f },
			{ 0.0f,						2.0f / -io.DisplaySize.y,	0.0f,	0.0f },
			{ 0.0f,						0.0f,						-1.0f,	0.0f },
			{ -1.0f,					1.0f,						0.0f,	1.0f },
		};
	
		glUseProgram(gShaderHandle);
		glUniform1i(gAttribLocationTex, 0);
		glUniformMatrix4fv(gAttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
		glBindVertexArray(gVaoHandle);
		glBindSampler(0, 0); // Rely on combined texture/sampler state.

		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = drawData->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			glBindBuffer(GL_ARRAY_BUFFER, gVboHandle);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementsHandle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
					glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
					glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
				}
				idx_buffer_offset += pcmd->ElemCount;
			}
		}

		// Restore scissor test
		opengl::enableScissorTest(false);
	}


	static const char* getClipboardText(void*)
	{
		return SDL_GetClipboardText();
	}


	static void setClipboardText(void*, const char* text)
	{
		SDL_SetClipboardText(text);
	}


	/**
	 * Creates font texture GPU resources
	 * TODO: Implement using own render objects
	 */
	static void createFontsTexture()
	{
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

		// Upload texture to graphics system
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGenTextures(1, &gFontTexture);
		glBindTexture(GL_TEXTURE_2D, gFontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Store our identifier
		io.Fonts->TexID = (void *)(intptr_t)gFontTexture;

		// Restore state
		glBindTexture(GL_TEXTURE_2D, last_texture);
	}


	/**
	* Deletes all GPU allocated resources
	* TODO: Implement using own render objects
	*/
	static void invalidateDeviceObjects()
	{
		if (!gFontTexture)
			return;

		glDeleteVertexArrays(1, &gVaoHandle);
		glDeleteBuffers(1, &gVboHandle);
		glDeleteBuffers(1, &gElementsHandle);
		gVaoHandle = gVboHandle = gElementsHandle = 0;

		glDetachShader(gShaderHandle, gVertHandle);
		glDeleteShader(gVertHandle);

		glDetachShader(gShaderHandle, gFragHandle);
		glDeleteShader(gFragHandle);
		gFragHandle = 0;

		glDeleteProgram(gShaderHandle);
		gShaderHandle = 0;

		glDeleteTextures(1, &gFontTexture);
		ImGui::GetIO().Fonts->TexID = 0;
		gFontTexture = 0;
	}


	/**
	 * Creates all GPU allocated resources
	 * TODO: Implement using own render objects
	 */
	static bool createDeviceObjects()
	{
		// Invalidate current GL resources
		invalidateDeviceObjects();

		// Backup GL state
		GLint last_texture, last_array_buffer, last_vertex_array;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

		const GLchar *vertex_shader =
			"#version 330\n"
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"	Frag_UV = UV;\n"
			"	Frag_Color = Color;\n"
			"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader =
			"#version 330\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
			"}\n";

		gShaderHandle = glCreateProgram();
		gVertHandle = glCreateShader(GL_VERTEX_SHADER);
		gFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(gVertHandle, 1, &vertex_shader, 0);
		glShaderSource(gFragHandle, 1, &fragment_shader, 0);
		glCompileShader(gVertHandle);
		glCompileShader(gFragHandle);
		glAttachShader(gShaderHandle, gVertHandle);
		glAttachShader(gShaderHandle, gFragHandle);
		glLinkProgram(gShaderHandle);

		gAttribLocationTex = glGetUniformLocation(gShaderHandle, "Texture");
		gAttribLocationProjMtx = glGetUniformLocation(gShaderHandle, "ProjMtx");
		gAttribLocationPosition = glGetAttribLocation(gShaderHandle, "Position");
		gAttribLocationUV = glGetAttribLocation(gShaderHandle, "UV");
		gAttribLocationColor = glGetAttribLocation(gShaderHandle, "Color");

		glGenBuffers(1, &gVboHandle);
		glGenBuffers(1, &gElementsHandle);

		glGenVertexArrays(1, &gVaoHandle);
		glBindVertexArray(gVaoHandle);
		glBindBuffer(GL_ARRAY_BUFFER, gVboHandle);
		glEnableVertexAttribArray(gAttribLocationPosition);
		glEnableVertexAttribArray(gAttribLocationUV);
		glEnableVertexAttribArray(gAttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
		glVertexAttribPointer(gAttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(gAttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(gAttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

		createFontsTexture();

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

		return true;
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
		if (SDL_GetWindowFlags(window.getNativeWindow()) & SDL_WINDOW_MOUSE_FOCUS)
			io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		else
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

		io.MouseDown[0] = gMousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[1] = gMousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2] = gMousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		gMousePressed[0] = gMousePressed[1] = gMousePressed[2] = false;

		io.MouseWheel = gMouseWheel;
		gMouseWheel = 0.0f;

		// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
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
		style.Colors[ImGuiCol_ChildWindowBg]			= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_PopupBg]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_Border]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_BorderShadow]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_FrameBg]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_FrameBgHovered]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_FrameBgActive]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBg]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBgCollapsed]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_TitleBgActive]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_MenuBarBg]				= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ScrollbarBg]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_ScrollbarGrab]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ScrollbarGrabHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ScrollbarGrabActive]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ComboBg]					= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_CheckMark]				= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_SliderGrab]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_SliderGrabActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_Button]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ButtonHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ButtonActive]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_Header]					= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_HeaderHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_HeaderActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_Column]					= IMGUI_NAPBACK;
		style.Colors[ImGuiCol_ColumnHovered]			= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ColumnActive]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_ResizeGrip]				= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ResizeGripHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_ResizeGripActive]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_CloseButton]				= IMGUI_NAPDARK;
		style.Colors[ImGuiCol_CloseButtonHovered]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_CloseButtonActive]		= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_PlotLines]				= IMGUI_NAPFRO2;
		style.Colors[ImGuiCol_PlotLinesHovered]			= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_PlotHistogram]			= IMGUI_NAPFRO3;
		style.Colors[ImGuiCol_PlotHistogramHovered]		= IMGUI_NAPHIGH;
		style.Colors[ImGuiCol_TextSelectedBg]			= IMGUI_NAPFRO1;
		style.Colors[ImGuiCol_ModalWindowDarkening]		= IMGUI_NAPBACK;
	}


	IMGuiService::IMGuiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	void IMGuiService::draw()
	{
		ImGui::Render();
	}


	void IMGuiService::selectWindow(nap::ResourcePtr<RenderWindow> window)
	{
		assert(window != nullptr);
		mUserWindow = window;
		setGuiWindow(window->getWindow()->getNativeWindow());
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
			switch (press_event.mButton)
			{
			case EMouseButton::LEFT:
				gMousePressed[0] = true;
				break;
			case EMouseButton::MIDDLE:
				gMousePressed[1] = true;
				break;
			case EMouseButton::RIGHT:
				gMousePressed[2] = true;
				break;
			default:
				break;
			}
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
		mRenderer = getCore().getService<nap::RenderService>();
		assert(mRenderer != nullptr);
	
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

		// initialize IMGUI, most important part here is to assign render callback
		io.RenderDrawListsFn  = renderDrawLists;
		io.SetClipboardTextFn = setClipboardText;
		io.GetClipboardTextFn = getClipboardText;
		io.ClipboardUserData = NULL;

		// Push default style
		applyStyle();

		// Add font
		ImFontConfig font_config;
		font_config.OversampleH = 8;
		font_config.OversampleV = 1;
		io.Fonts->AddFontFromMemoryCompressedTTF(nunitoSansSemiBoldData, nunitoSansSemiBoldSize, 17.0f, &font_config);

		// Set primary window to be default
		setGuiWindow(mRenderer->getPrimaryWindow().getNativeWindow());

		return true;
	}


	void IMGuiService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
		dependencies.emplace_back(RTTI_OF(RenderService));
	}


	void IMGuiService::update(double deltaTime)
	{
		// Activate correct context
		GLWindow* current_window = mUserWindow != nullptr ? mUserWindow->getWindow() : &(mRenderer->getPrimaryWindow());
		current_window->makeCurrent();

		// Create new device objects
		if (mWindowChanged)
		{
			createDeviceObjects();
			mWindowChanged = false;
		}

		// Create new GUI frame
		newFrame(*current_window);
	};


	void IMGuiService::shutdown()
	{
		invalidateDeviceObjects();
		ImGui::Shutdown();
	}
}
