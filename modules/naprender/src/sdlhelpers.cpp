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
		bool initVideo()
		{
			// Initialize SDL's Video subsystem
			if (SDL_Init(SDL_INIT_VIDEO) < 0)
				return false;

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
			std::uint32_t full_screen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
			std::uint32_t flag = value ? full_screen_flag : 0;
			SDL_SetWindowFullscreen(window, flag);
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
			// Ensure sizes are not the same
			glm::ivec2 wsize = getWindowSize(window);
			if (wsize == size)
				return;
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


		// Returns an SDL window based on the given id
		SDL_Window* getWindow(uint32_t  id)
		{
			return SDL_GetWindowFromID((Uint32)(id));
		}


		uint32_t getWindowId(SDL_Window* window)
		{
			return SDL_GetWindowID(window);
		}


		void hideCursor()
		{
			SDL_ShowCursor(SDL_DISABLE);
		}


		void showCursor()
		{
			SDL_ShowCursor(SDL_ENABLE);
		}


		bool getFullscreen(SDL_Window* window)
		{
			uint32 flags = SDL_GetWindowFlags(window);
			return flags & SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
	}
}
