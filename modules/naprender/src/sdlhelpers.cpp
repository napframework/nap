/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "sdlhelpers.h"

// External Includes
#include <assert.h>
#include <SDL.h>
#include <SDL_vulkan.h>

namespace nap
{
	namespace SDL
	{
		// Initializes SDL's video subsystem
		bool initVideo(utility::ErrorState& error)
		{
			// Initialize SDL's Video subsystem
			if (SDL_Init(SDL_INIT_VIDEO) < 0)
			{
				error.fail(SDL_GetError());
				return false;
			}
			return true;
		}


		void setWindowBordered(SDL_Window* window, bool hasBorders)
		{
			SDL_SetWindowBordered(window, (SDL_bool)hasBorders);
		}


		void setWindowTitle(SDL_Window* window, const std::string& name)
		{
			SDL_SetWindowTitle(window, name.c_str());
		}


		void showWindow(SDL_Window* window, bool show)
		{
			if (show)
			{
				SDL_ShowWindow(window);
				return;
			}
			SDL_HideWindow(window);
		}


		void raiseWindow(SDL_Window* window)
		{
			SDL_RaiseWindow(window);
		}


		void setFullscreen(SDL_Window* window, bool value)
		{
			// Otherwise set
			uint32 full_screen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
			SDL_SetWindowFullscreen(window, value ? full_screen_flag : 0);
		}


		glm::ivec2 getWindowSize(SDL_Window* window)
		{
			int x, y;
			SDL_GetWindowSize(window, &x, &y);
			return glm::ivec2(x, y);
		}


		uint32 NAPAPI getWindowFlags(SDL_Window* window)
		{
			return SDL_GetWindowFlags(window);
		}


		glm::ivec2 getScreenSize(int screenIndex)
		{
			SDL_DisplayMode mode;
			int v = SDL_GetDesktopDisplayMode(screenIndex, &mode);
			return v == 0 ? glm::ivec2(mode.w, mode.h) : glm::ivec2(-1, -1);
		}


		void setWindowSize(SDL_Window* window, const glm::ivec2& size)
		{
			SDL_SetWindowSize(window, size.x, size.y);
		}


		glm::ivec2 getDrawableWindowSize(SDL_Window* window)
		{
			int x, y;
			SDL_Vulkan_GetDrawableSize(window, &x, &y);
			return{ x, y };
		}


		glm::ivec2 getWindowPosition(SDL_Window* window)
		{
			int x, y;
			SDL_GetWindowPosition(window, &x, &y);
			return{ x, y };
		}


		void setWindowPosition(SDL_Window* window, const glm::ivec2& position)
		{
			// Ensure position is not the same
			glm::ivec2 wpos = getWindowPosition(window);
			if (position == wpos)
				return;
			// Update position
			SDL_SetWindowPosition(window, position.x, position.y);
		}


		// Shuts down SDL, make sure to 
		void shutdownVideo()
		{
			SDL_Quit();
		}


		// Returns the last SDL error
		std::string getSDLError()
		{
			return std::string(SDL_GetError());
		}


		uint32_t getWindowId(SDL_Window* window)
		{
			return SDL_GetWindowID(window);
		}


		int getDisplayCount()
		{
			return SDL_GetNumVideoDisplays();
		}


		int NAPAPI getDisplayIndex(SDL_Window* window)
		{
			return SDL_GetWindowDisplayIndex(window);
		}


		int NAPAPI getDisplayDPI(int displayIndex, float* ddpi, float* hdpi, float* vdpi)
		{
			return SDL_GetDisplayDPI(displayIndex, ddpi, hdpi, vdpi);
		}


		int NAPAPI getDisplayDPI(SDL_Window* window, float* ddpi, float* hdpi, float* vdpi)
		{
			int idx = getDisplayIndex(window);
			return idx >= 0 ? getDisplayDPI(idx, ddpi, hdpi, vdpi) : idx;
		}


		bool getDisplayName(int displayIndex, std::string& outName)
		{
			const char* display_name = SDL_GetDisplayName(displayIndex);
			outName = display_name != nullptr ? display_name : "";
			return display_name != nullptr;
		}


		int getDisplayBounds(int displayIndex, glm::ivec2& outMin, glm::ivec2& outMax)
		{
			SDL_Rect sdl_bounds;
			int r = SDL_GetDisplayBounds(displayIndex, &sdl_bounds);
			outMin = { sdl_bounds.x, sdl_bounds.y };
			outMax = { sdl_bounds.x + sdl_bounds.w, sdl_bounds.y + sdl_bounds.h };
			return r;
		}


		void hideCursor()
		{
			SDL_ShowCursor(SDL_DISABLE);
		}


		void showCursor()
		{
			SDL_ShowCursor(SDL_ENABLE);
		}


		bool cursorVisible()
		{
			return SDL_ShowCursor(SDL_QUERY) > 0;
		}


		void toggleCursor()
		{
			cursorVisible() ? hideCursor() : showCursor();
		}


		glm::ivec2 getCursorPosition()
		{
			glm::ivec2 pos;
			SDL_GetMouseState(&pos.x, &pos.y);
			return pos;
		}


		uint32 getMouseState(int* x, int* y)
		{
			return SDL_GetMouseState(x, y);
		}


		glm::ivec2 getGlobalCursorPosition()
		{
			glm::ivec2 pos;
			SDL_GetGlobalMouseState(&pos.x, &pos.y);
			return pos;
		}


		uint32 getGlobalMouseState(int* x, int* y)
		{
			return SDL_GetGlobalMouseState(x, y);
		}


		bool getFullscreen(SDL_Window* window)
		{
			uint32 flags = SDL_GetWindowFlags(window);
			return flags & SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
	}
}
