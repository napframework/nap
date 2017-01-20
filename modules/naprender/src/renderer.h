#pragma once

#include <rtti/rtti.h>
#include <renderattributes.h>

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
		bool resizable  = true;					//< If the window is resizable
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

	protected:
		RenderWindowSettings mSettings;
	};


	/**
	 * Renderer Base Class
	 * Derived classed implement specific hardware rendering
	 * options. Every renderer renders a set of objects to 
	 * a specific render target. The render target initialization
	 * and binding is handled by that target
	 */
	class Renderer
	{
		RTTI_ENABLE()
	public:
		/**
		 * Delete copy constructor
		 */
		Renderer(const Renderer& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;

		/**
		 * Default constructor and destructor
		 */
		virtual ~Renderer() = default;
		Renderer() = default;

		/**
		 * Called before creating the first render context
		 * @return if the initialization call was successful
		 */
		virtual bool preInit() = 0;


		/**
		 * Called to create a window
		 * Create a window with associated render context
		 * @return the new render window or nullptr if unsuccessful
		 * @param settings the window settings used to create the window
		 */
		virtual RenderWindow* createRenderWindow(const RenderWindowSettings& settings) = 0;


		/**
		 * Called after the first render context is created
		 * @retrun if the initialization call was successful
		 */
		virtual bool postInit() = 0;


		/**
		 * Called when the renderer needs to shut down
		 */
		virtual void shutdown() = 0;
	};
}

RTTI_DECLARE_BASE(nap::Renderer)
RTTI_DECLARE_BASE(nap::RenderWindow)