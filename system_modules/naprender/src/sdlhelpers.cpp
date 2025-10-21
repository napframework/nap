/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "sdlhelpers.h"
#include "bitmap.h"

// External Includes
#include <assert.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <mathutils.h>
#include <rtti/typeinfo.h>
#include <utility/fileutils.h>

namespace nap
{
	namespace SDL
	{
		bool initVideo(utility::ErrorState& error)
		{
			bool initialized = SDL_Init(SDL_INIT_VIDEO);
			return error.check(initialized, SDL_GetError());
		}


		bool initVideo(EVideoDriver driver, utility::ErrorState& error)
		{
			// Use current selected default
			if (driver == EVideoDriver::Default)
			{
				// Reset if video selection is active
				if (SDL_GetHint(SDL_HINT_VIDEO_DRIVER) && !SDL_ResetHint(SDL_HINT_VIDEO_DRIVER))
				{
					error.fail("Unable to reset video driver to system default, error: ",
						SDL_GetError());
					return false;
				}
				return initVideo(error);
			}

			// Convert enum to driver name
			auto stype = RTTI_OF(EVideoDriver).get_enumeration(); assert(stype.is_valid());
			auto sname = utility::toLower(toString(driver));

			// Find compatible video driver
			auto vnames = getVideoDrivers();
			const auto& fname = std::find_if(vnames.begin(), vnames.end(), [&sname](const auto& vname)
				{
					return vname == sname;
				});

			// Make sure the selected driver is supported on the current platform
			if(!error.check(fname != vnames.end(),
				"Selected video driver '%s' not supported", sname.c_str()))
				return false;

			// Select it
			bool selected = SDL_SetHint(SDL_HINT_VIDEO_DRIVER, sname.c_str());
			if (!error.check(selected, "Unable to select '%s' video driver, error: '%s'",
				sname.c_str(), SDL_GetError()))
				return false;

			// Initialize video
			return initVideo(error);
		}


		bool videoInitialized()
		{
			return SDL_WasInit(SDL_INIT_VIDEO) > 0;
		}


		void setWindowBordered(SDL_Window* window, bool hasBorders)
		{
			SDL_SetWindowBordered(window, hasBorders);
		}


		void setWindowTitle(SDL_Window* window, const std::string& name)
		{
			SDL_SetWindowTitle(window, name.c_str());
		}


		void setWindowAlwaysOnTop(SDL_Window* window, bool enabled)
		{
			SDL_SetWindowAlwaysOnTop(window, enabled);
		}


		void setWindowResizable(SDL_Window* window, bool enabled)
		{
			SDL_SetWindowResizable(window, enabled);
		}


		SurfacePtr createSurface(const std::string& imagePath, utility::ErrorState& error)
		{
			// Ensure file exists
			if (!error.check(utility::fileExists(imagePath),
				"File '%s' doesn't exist", imagePath.c_str()))
				return nullptr;

			// Load as bitmap
			BitmapFromFile bitmap;
			bitmap.mPath = imagePath;
			if (!bitmap.init(error))
				return nullptr;

			// Cache some conversion variables
			auto src_color = bitmap.makePixel(); RGBAColor8 dst_color;
			auto cchannels = math::min<int>(src_color->getNumberOfChannels(), dst_color.getNumberOfChannels());
			auto usepalpha = dst_color.getNumberOfChannels() == 4;

			// Pre-load conversion function -> prevents unnecessary table lookups
			auto converter = src_color->getConverter(dst_color);
			if (!error.check(converter != nullptr, "Unable to find compatible color converter"))
				return nullptr;

			// Create SDL surface
			auto w = bitmap.getWidth(); auto h = bitmap.getHeight();
			auto s = SurfacePtr(SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888), [](SDL_Surface* surface)
				{
					SDL_DestroySurface(surface);
				});

			// Ensure it exists
			if (!error.check(s != nullptr, "Unable to create icon surface: %s", SDL_GetError()))
				return nullptr;

			// Convert and copy into place
			assert(s->w == w && s->h == h);
			for (auto x = 0; x < w; x++)
			{
				for (auto y = 0; y < h; y++)
				{
					// Sample pixel and convert to target
					bitmap.getPixel(x, y, *src_color);
					for (auto i = 0; i < cchannels; i++)
						converter(*src_color, dst_color, i);

					// Write
					if (!SDL_WriteSurfacePixel(s.get(), x, y,
						dst_color[0],
						dst_color[1],
						dst_color[2],
						usepalpha ? dst_color[3] : math::max<uint8>()))
					{
						error.fail("Pixel write failed, %s", SDL::getSDLError().c_str());
						return nullptr;
					}
				}
			}

			// Store handle and return
			return std::move(s);
		}


		bool NAPAPI enableTextInput(SDL_Window* window)
		{
			return SDL_StartTextInput(window);
		}


		bool NAPAPI disableTextInput(SDL_Window* window)
		{
			return SDL_StopTextInput(window);
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


		bool setFullscreen(SDL_Window* window, bool value)
		{
			// Otherwise set
			return SDL_SetWindowFullscreen(window, value);
		}


		glm::ivec2 getWindowSize(SDL_Window* window)
		{
			int x, y;
			SDL_GetWindowSize(window, &x, &y);
			return glm::ivec2(x, y);
		}


		uint32 getWindowFlags(SDL_Window* window)
		{
			return SDL_GetWindowFlags(window);
		}


		glm::ivec2 getScreenSize(int screenIndex)
		{
			const auto* mode = SDL_GetDesktopDisplayMode(static_cast<SDL_DisplayID>(screenIndex));
			return mode != nullptr ? glm::ivec2(mode->w, mode->h) : glm::ivec2(-1, -1);
		}


		void setWindowSize(SDL_Window* window, const glm::ivec2& size)
		{
			SDL_SetWindowSize(window, size.x, size.y);
		}


		glm::ivec2 getDrawableWindowSize(SDL_Window* window)
		{
			int x, y;
			SDL_GetWindowSizeInPixels(window, &x, &y);
			return{ x, y };
		}


		glm::ivec2 getWindowPosition(SDL_Window* window)
		{
			int x, y;
			SDL_GetWindowPosition(window, &x, &y);
			return{ x, y };
		}


		bool setWindowPosition(SDL_Window* window, const glm::ivec2& position)
		{
			// Ensure position is not the same
			glm::ivec2 wpos = getWindowPosition(window);
			if (position == wpos)
				return true;

			// Update position
			return SDL_SetWindowPosition(window, position.x, position.y);
		}


		// Shuts down SDL, make sure to 
		void shutdownVideo()
		{
			SDL_Quit();
		}


		// Returns the last SDL error
		std::string getSDLError()
		{
			return SDL_GetError();
		}


		uint32_t getWindowId(SDL_Window* window)
		{
			return SDL_GetWindowID(window);
		}


		int getDisplayCount()
		{
			return getDisplayIDs().size();
		}


		std::vector<int> getDisplayIDs()
		{
			// Get unique display id
			int count; int* displays = reinterpret_cast<int*>(SDL_GetDisplays(&count));
			if (displays == nullptr || count == 0)
				return {};

			// Create copy and free previous list
			std::vector<int> ids(displays, displays + count);
			SDL_free(displays);
			return ids;
		}


		std::vector<nap::Display> getDisplays()
		{
			// Get unique display id
			int count; int* ids = reinterpret_cast<int*>(SDL_GetDisplays(&count));
			if (ids == nullptr || count == 0)
				return {};

			// Create display list
			std::vector<nap::Display> dl; dl.reserve(count);
			for (auto i = 0; i < count; i++)
				dl.emplace_back(nap::Display(ids[i]));

			SDL_free(ids);
			return dl;
		}


		int getDisplayIndex(SDL_Window* window)
		{
			auto idx = SDL_GetDisplayForWindow(window);
			return idx == 0 ? -1 : idx;
		}


		bool getDisplayDPI(int displayIndex, float* dpi)
		{
			bool valid = getDisplayContentScale(displayIndex, dpi);
			*dpi *= 96.0f;
			return valid;
		}


		bool getDisplayDPI(SDL_Window* window, float* dpi)
		{
			int idx = getDisplayIndex(window);
			return idx >= 0 ? getDisplayDPI(idx, dpi) : false;
		}


		bool getDisplayContentScale(int displayIndex, float* scale)
		{
			*scale = SDL_GetDisplayContentScale(static_cast<SDL_DisplayID>(displayIndex));
			return *scale > nap::math::epsilon<float>();
		}


		bool getDisplayContentScale(SDL_Window* window, float* scale)
		{
			auto idx = getDisplayIndex(window);
			if (idx < 0)
			{
				*scale = 0.0f;
				return false;
			}
			return getDisplayContentScale(idx, scale);
		}


		Display::EOrientation getDisplayOrientation(int displayIndex)
		{
			switch (SDL_GetCurrentDisplayOrientation(displayIndex))
			{
			case SDL_ORIENTATION_UNKNOWN:
				return Display::EOrientation::Unknown;
			case SDL_ORIENTATION_LANDSCAPE:
				return Display::EOrientation::Landscape;
			case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
				return Display::EOrientation::LandscapeFlipped;
			case SDL_ORIENTATION_PORTRAIT:
				return Display::EOrientation::Portrait;
			case SDL_ORIENTATION_PORTRAIT_FLIPPED:
				return Display::EOrientation::PortraitFlipped;
			default:
				assert(false);
				break;
			}
			return Display::EOrientation::Unknown;
		}


		bool getWindowPixelDensity(SDL_Window* window, float* density)
		{
			*density = SDL_GetWindowPixelDensity(window);
			return *density > nap::math::epsilon<float>();
		}


		bool getWindowDisplayScale(SDL_Window* window, float* scale)
		{
			*scale = SDL_GetWindowDisplayScale(window);
			return *scale > nap::math::epsilon<float>();
		}


		bool getDisplayName(int displayIndex, std::string& outName)
		{
			const char* display_name = SDL_GetDisplayName(static_cast<SDL_DisplayID>(displayIndex));
			outName = display_name != nullptr ? display_name : "";
			return display_name != nullptr;
		}


		bool getDisplayBounds(int displayIndex, glm::ivec2& outMin, glm::ivec2& outMax)
		{
			SDL_Rect sdl_bounds;
			if (SDL_GetDisplayBounds(displayIndex, &sdl_bounds))
			{
				outMin = { sdl_bounds.x, sdl_bounds.y };
				outMax = { sdl_bounds.x + sdl_bounds.w, sdl_bounds.y + sdl_bounds.h };
				return true;
			}
			return false;
		}


		void hideCursor()
		{
			SDL_HideCursor();
		}


		void showCursor()
		{
			SDL_ShowCursor();
		}


		bool cursorVisible()
		{
			return SDL_CursorVisible();
		}


		void toggleCursor()
		{
			cursorVisible() ? hideCursor() : showCursor();
		}


		glm::vec2 getCursorPosition()
		{
			glm::vec2 pos;
			SDL_GetMouseState(&pos.x, &pos.y);
			return pos;
		}


		uint32 getMouseState(float* x, float* y)
		{
			return SDL_GetMouseState(x, y);
		}


		glm::vec2 getGlobalCursorPosition()
		{
			glm::vec2 pos;
			SDL_GetGlobalMouseState(&pos.x, &pos.y);
			return pos;
		}


		uint32 getGlobalMouseState(float* x, float* y)
		{
			return SDL_GetGlobalMouseState(x, y);
		}


		bool getFullscreen(SDL_Window* window)
		{
			uint32 flags = SDL_GetWindowFlags(window);
			return flags & SDL_WINDOW_FULLSCREEN;
		}


		std::vector<std::string> getVideoDrivers()
		{
			int count = SDL_GetNumVideoDrivers();
			std::vector<std::string> drivers; drivers.reserve(count);
			for (auto i = 0; i < count; i++)
				drivers.emplace_back(SDL_GetVideoDriver(i));
			return drivers;
		}


		std::string getCurrentVideoDriver()
		{
			const auto* sdl_driver = SDL_GetCurrentVideoDriver();
			return sdl_driver != nullptr ? sdl_driver : std::string();
		}
	}
}
