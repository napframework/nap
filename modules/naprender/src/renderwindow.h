#pragma once

// External Includes
#include <rtti/rtti.h>
#include <string.h>
#include <glm/glm.hpp>

namespace nap
{
	// Forward Declares
	class RenderWindow;

	/**
	* Holds all window launch settings
	* Note that this object is only used when constructing the window
	* Use the size, position and title attributes on the component
	* to position the window
	*/
	struct RenderWindowSettings
	{
	public:
		// Construction / Destruction
		RenderWindowSettings() = default;
		virtual ~RenderWindowSettings() = default;

		bool borderless = false;				//< If the window is borderless
		bool resizable = true;					//< If the window is resizable
		RenderWindow* sharedWindow = nullptr;	//< If the window shares a context with another window
	};


	/**
	* Render window base class
	* This is just an empty shell that is used to identify a window
	*/
	class RenderWindow
	{
		RTTI_ENABLE()
	public:
		/**
		* Don't allow default construction
		*/
		RenderWindow() = delete;

		/**
		* Default destruction
		*/
		virtual ~RenderWindow() = default;

		/**
		* Only construct using window settings
		*/
		RenderWindow(const RenderWindowSettings& settings) :
			mSettings(settings) {}

		/**
		* Delete copy construction
		*/
		RenderWindow(const RenderWindow& other) = delete;
		RenderWindow& operator=(const RenderWindow& other) = delete;

		/**
		* @return the hardware window handle, nullptr if undefined
		*/
		virtual void* getWindow() const = 0;

		/**
		* @return the hardware window context, nullptr if undefined
		*/
		virtual void* getContext() const = 0;

		/**
		 * Set the window title
		 * @param title the new window title
		 */
		virtual void setTitle(const std::string& title) = 0;

		/**
		 * Set the window position
		 * @param position the window position coordinates in pixels
		 */
		virtual void setPosition(const glm::ivec2& position) = 0;

		/**
		 * Set the window size
		 * @param size the new window size in pixels
		 */
		virtual void setSize(const glm::ivec2& size) = 0;

		/**
		 * Turns v-sync on / off
		 * @param value if v-sync should be turned on or off
		 */
		virtual void setSync(bool value) = 0;

		/**
		 * Show window
		 */
		virtual void showWindow() = 0;

		/**
		 * Hide window
		 */
		virtual void hideWindow() = 0;

		/**
		 * Swap buffers
		 */
		virtual void swap() = 0;

		/**
		 * Make this window active
		 */
		virtual void makeCurrent() = 0;

	protected:
		RenderWindowSettings mSettings;
	};
}

RTTI_DECLARE_BASE(nap::RenderWindow)