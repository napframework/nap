#pragma once

// External Includes
#include <nap/attribute.h>
#include <nap/service.h>
#include <nopengl.h>
#include <thread>
#include <nwindow.h>

namespace nap
{
	// Forward Declares
	class RenderWindowComponent;


	/**
	 * Holds a reference to all drawable objects
	 */
	class RenderService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)

	public:
		RenderService() = default;

		/**
		 * Init needs to be called on the render service to initialize opengl
		 * related calls and functionality. Failure to do so will result in undefined behavior 
		 * when dealing with opengl state based objects
		 */
		void init();

		/**
		 * Call this in your app loop to emit a render call
		 * When called the draw bang is emitted
		 */
		void render();

		/**
		 * The draw signal that is emitted every render call
		 * Register to this event to receive draw calls
		 */
		SignalAttribute draw = {this, "draw"};

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
		bool glewInitialized = false;
	};
} // nap

RTTI_DECLARE(nap::RenderService)
