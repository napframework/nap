#pragma once

// Local Includes
#include <renderwindow.h>

// External Includes
#include <rtti/rtti.h>
#include <renderattributes.h>

namespace nap
{
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