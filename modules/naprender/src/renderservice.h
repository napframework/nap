#pragma once

// External Includes
#include <nap/attribute.h>
#include <nap/service.h>
#include <nap/timer.h>
#include <nopengl.h>
#include <thread>
#include <nwindow.h>

// Local Includes
#include "renderer.h"

namespace nap
{
	// Forward Declares
	class RenderWindowComponent;
	class TransformComponent;

	/**
	 * Holds a reference to all drawable objects
	 */
	class RenderService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)

	public:
		/**
		 * Holds current render state 
		 */
		enum class State : int
		{
			Uninitialized		= -1,
			Initialized			= 0,
			WindowError			= 1,
			SystemError			= 2,
		};

		// Default constructor
		RenderService() = default;

		// Default destructor
		virtual ~RenderService();

		/**
		 * Call this in your app loop to emit a render call
		 * Note that this call will gather all available windows
		 * and for every window call it's update and render signals
		 * Subscribe to those signals to update app members and
		 * render objects to a specific target
		 */
		void render();

		/**
		* Call this to update all transform components
		* This call is also called when rendering
		*/
		void updateTransforms();

		/**
		 * Renders all available objects to currently active buffer
		 * TODO: deprecate
		 */
		void renderObjects();

		/**
		 * @return if OpenGL has been initialized
		 */
		bool isInitialized() const									{ return state == State::Initialized; }

		/**
		 * Sets the renderer, the service will own the renderer
		 * @param renderer the type of renderer to use
		 */
		void setRenderer(const RTTI::TypeInfo& renderer);

		/**
		 * Shuts down the managed renderer
		 */
		void shutdown();

		/**
		 * @return number of elapsed time in milliseconds after
		 * creation of render window and context
		 */
		uint32 getTicks();

		/**
		 * @return number of elapsed seconds after creation of 
		 * render window and context
		 */
		double getElapsedTime();

	protected:
		/**
		 * Type registration
		 */
		virtual void registerTypes(nap::Core& core) override;

		/**
		 * Occurs when an object registers itself with the service
		 */
		virtual void objectRegistered(Object& inObject) override;

    private:

		// Keeps track of glew initialization
		// If there is no active context we can't initialize glew
		// This 
		State state = State::Uninitialized;

		/**
		 * Creates a new window and assigns it to the window component
		 * Note that this call implicitly initializes OpenGL
		 */
		void createWindow(RenderWindowComponent& window);

		/**
		 * Holds the currently active renderer
		 */
		std::unique_ptr<nap::Renderer> mRenderer = nullptr;

		/**
		 * Timer that keeps track of elapsed render time
		 */
		SimpleTimer mTimer;

		/**
		 * Finds all top level transforms, this is a recursive function
		 * If an entity has a transform it will be considered to be at the top of the chain
		 * if not, all subsequent children will be checked, until the top most level xforms have 
		 * been found
		 * @param entity, entity to check for transform component
		 * @param xforms the total amount of top level transforms
		 */
		void getTopLevelTransforms(Entity* entity, std::vector<TransformComponent*>& xforms);
	};
} // nap

RTTI_DECLARE(nap::RenderService)
