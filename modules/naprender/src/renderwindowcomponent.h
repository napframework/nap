#pragma once

// Local Includes
#include "renderservice.h"

// External Includes
#include <nap.h>

namespace nap
{
	/**
	 * 3D render window. Creating a window requests a new 3d window
	 * with context from the Render Service. Every window is associated
	 * with it's own drawing context. Note that this window does not own
	 * the OpenGL render window, it acts as an interface to the window managed 
	 * by the service
	 */
	class RenderWindowComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		/**
		 * Constructor
		 */
		RenderWindowComponent();

		void createWindow();

		void destroyWindow();

		void hideWindow() { destroyWindow(); }

		void showWindow() { createWindow(); }

		Attribute<bool> visible = {this, "visible", true};

	protected:
		// Callable Slots
		void onAdded(Object& parent)					{ createWindow(); }
		void onRemoved(Object& parent)					{ destroyWindow(); }
		void onVisibilityChanged(const bool& value);

		// Slot declarations
		NSLOT(componentAdded,	Object&, onAdded)
		NSLOT(componentRemoved,	Object&, onRemoved)
		NSLOT(visibilityChanged, const bool&, onVisibilityChanged)


	private:
		opengl::Window* mWindow = nullptr;
	};
}

RTTI_DECLARE(nap::RenderWindowComponent)