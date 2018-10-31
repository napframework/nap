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

// Data
static double       g_Time = 0.0f;
static bool         g_MousePressed[3] = { false, false, false };
static float        g_MouseWheel = 0.0f;
static GLuint       g_FontTexture = 0;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

namespace nap
{
	// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
	// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so. 
	// If text or lines are blurry when integrating ImGui in your engine: in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
	static void renderDrawLists(ImDrawData* drawData)
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO& io = ImGui::GetIO();
		int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
			return;
		drawData->ScaleClipRects(io.DisplayFramebufferScale);

		// Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
		GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
		GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
		GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
		GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
		GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
		GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
		GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
		GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
		GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
		GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
		GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
		GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
		GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Setup viewport, orthographic projection matrix
		glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
		const float ortho_projection[4][4] =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
			{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
			{ -1.0f,                  1.0f,                   0.0f, 1.0f },
		};
		glUseProgram(g_ShaderHandle);
		glUniform1i(g_AttribLocationTex, 0);
		glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
		glBindVertexArray(g_VaoHandle);
		glBindSampler(0, 0); // Rely on combined texture/sampler state.

		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = drawData->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
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

		// Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindSampler(0, last_sampler);
		glActiveTexture(last_active_texture);
		glBindVertexArray(last_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, last_polygon_mode[0]);
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
	}


	static const char* getClipboardText(void*)
	{
		return SDL_GetClipboardText();
	}


	static void setClipboardText(void*, const char* text)
	{
		SDL_SetClipboardText(text);
	}


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
		glGenTextures(1, &g_FontTexture);
		glBindTexture(GL_TEXTURE_2D, g_FontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Store our identifier
		io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

		// Restore state
		glBindTexture(GL_TEXTURE_2D, last_texture);
	}


	static bool createDeviceObjects()
	{
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

		g_ShaderHandle = glCreateProgram();
		g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
		g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
		glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
		glCompileShader(g_VertHandle);
		glCompileShader(g_FragHandle);
		glAttachShader(g_ShaderHandle, g_VertHandle);
		glAttachShader(g_ShaderHandle, g_FragHandle);
		glLinkProgram(g_ShaderHandle);

		g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
		g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
		g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
		g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
		g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

		glGenBuffers(1, &g_VboHandle);
		glGenBuffers(1, &g_ElementsHandle);

		glGenVertexArrays(1, &g_VaoHandle);
		glBindVertexArray(g_VaoHandle);
		glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
		glEnableVertexAttribArray(g_AttribLocationPosition);
		glEnableVertexAttribArray(g_AttribLocationUV);
		glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
		glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

		createFontsTexture();

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

		return true;
	}


	static void invalidateDeviceObjects()
	{
		if (g_VaoHandle) glDeleteVertexArrays(1, &g_VaoHandle);
		if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
		if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
		g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

		if (g_ShaderHandle && g_VertHandle) glDetachShader(g_ShaderHandle, g_VertHandle);
		if (g_VertHandle) glDeleteShader(g_VertHandle);
		g_VertHandle = 0;

		if (g_ShaderHandle && g_FragHandle) glDetachShader(g_ShaderHandle, g_FragHandle);
		if (g_FragHandle) glDeleteShader(g_FragHandle);
		g_FragHandle = 0;

		if (g_ShaderHandle) glDeleteProgram(g_ShaderHandle);
		g_ShaderHandle = 0;

		if (g_FontTexture)
		{
			glDeleteTextures(1, &g_FontTexture);
			ImGui::GetIO().Fonts->TexID = 0;
			g_FontTexture = 0;
		}
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


	static void newFrame(SDL_Window* window)
	{
		if (!g_FontTexture)
			createDeviceObjects();

		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		SDL_GetWindowSize(window, &w, &h);
		SDL_GL_GetDrawableSize(window, &display_w, &display_h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

		// Setup time step
		Uint32	time = SDL_GetTicks();
		double current_time = time / 1000.0;
		io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
		g_Time = current_time;

		// Setup inputs
		// (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
		int mx, my;
		Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
			io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		else
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

		io.MouseDown[0] = g_MousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[1] = g_MousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2] = g_MousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

		io.MouseWheel = g_MouseWheel;
		g_MouseWheel = 0.0f;

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


	void IMGuiService::setWindow(nap::ResourcePtr<RenderWindow> window)
	{
		assert(window != nullptr);
		mUserWindow = window;
		setGuiWindow(window->getWindow()->getNativeWindow());
	}


	void IMGuiService::processInputEvent(opengl::Event& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
		{
			if (event.wheel.y > 0)
				g_MouseWheel = 1;
			if (event.wheel.y < 0)
				g_MouseWheel = -1;
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if (event.button.button == SDL_BUTTON_LEFT) g_MousePressed[0] = true;
			if (event.button.button == SDL_BUTTON_RIGHT) g_MousePressed[1] = true;
			if (event.button.button == SDL_BUTTON_MIDDLE) g_MousePressed[2] = true;
			break;
		}
		case SDL_TEXTINPUT:
		{
			io.AddInputCharactersUTF8(event.text.text);
			break;
		}
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			int key = event.key.keysym.sym & ~SDLK_SCANCODE_MASK;
			io.KeysDown[key]	= (event.type == SDL_KEYDOWN);
			io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
			io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
			io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
			io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
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
	
		// Imgui global state object
		ImGuiIO& io = ImGui::GetIO();

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
		io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
		io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
		io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
		io.KeyMap[ImGuiKey_A] = SDLK_a;
		io.KeyMap[ImGuiKey_C] = SDLK_c;
		io.KeyMap[ImGuiKey_V] = SDLK_v;
		io.KeyMap[ImGuiKey_X] = SDLK_x;
		io.KeyMap[ImGuiKey_Y] = SDLK_y;
		io.KeyMap[ImGuiKey_Z] = SDLK_z;

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
		if (mUserWindow != nullptr)
		{	
			mUserWindow->makeActive();
			newFrame(mUserWindow->getWindow()->getNativeWindow());
			return;
		}

		mRenderer->getPrimaryWindow().makeCurrent();
		newFrame(mRenderer->getPrimaryWindow().getNativeWindow());
	};


	void IMGuiService::shutdown()
	{
		invalidateDeviceObjects();
		ImGui::Shutdown();
	}
}