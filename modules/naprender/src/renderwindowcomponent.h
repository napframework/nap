#pragma once

// Local Includes
#include "renderservice.h"
#include "renderattributes.h"

// External Includes
#include <nap.h>

namespace nap
{
	/**
	 * Render window. 
	 * When adding this object to an entity a new render window is created
	 * If you want to change the window settings on construction don't add
	 * it immediately, create this component without adding it, change the
	 * window settings and attach to it's parent. The render service
	 * will handle all opengl related initialization calls and creates
	 * the actual window. The window is managed by this component and 
	 * destroyed upon removal Every window is associated
	 * with it's own drawing context. Note that the window this object
	 * manages can be of any type, ie: opengl, direct3d etc.
	 */
	class RenderWindowComponent : public ServiceableComponent
	{
		friend class RenderService;
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		/**
		 * Constructor
		 */
		RenderWindowComponent();

		/**
		 * By default the window is not visible
		 * By making the window visible for the first time the window
		 * is constructed
		 */
		SignalAttribute show { this, "Show" };

		/**
		 * Hides the window
		 */
		SignalAttribute hide { this, "Hide" };

		/**
		 * Render signal, emitted every render iteration
		 * Connect to this signal to render objects to the context
		 * associated with this window. 
		 */
		SignalAttribute render { this, "Render" };

		/**
		* These attributes only work when the window has
		* been registered with the render service
		*/
		Attribute<glm::ivec2> position{ this, "Position",{ 256, 256 } };
		Attribute<glm::ivec2> size{ this, "Size",{ 512, 512 } };
		Attribute<std::string> title{ this, "Title", "RenderWindow" };
		Attribute<bool> sync{ this, "VSync", false };

		/**
		 * @return if the component manages a window. 
		 * If show hasn't been called this call will resolve to false
		 */
		bool hasWindow() const													{ return mWindow != nullptr; }

		/**
		 * Swaps window buffers
		 */
		void swap() const														{ mWindow->swap(); }

		/**
		 * Sets window construction settings
		 * These settings are used when the window is constructed
		 */
		void setConstructionSettings(const RenderWindowSettings& settings);

		/**
		 * Returns window construction settings
		 */
		const RenderWindowSettings& getConstructionSettings() const				{ return mSettings; }

	protected:
		/**
		 * Creates the window
		 */
		void onAdded(Object& parent);

		/*
		* Hides / shows the window
		*/
		void onShowWindow(const SignalAttribute& signal);
		void onHideWindow(const SignalAttribute& signal);

		/**
		 * Attribute changes
		 */
		void onTitleChanged(const std::string& title);
		void onPositionChanged(const glm::ivec2& position);
		void onSizeChanged(const glm::ivec2& size);
		void onSyncChanged(const bool& value);

		/**
		 * Occurs when the window is registered with the render service
		 * This call is necessary because of opengl initialization order
		 * the service will provide this component with a window and valid opengl context
		 * when on construction attributes are changed the window might not be available
		 * this call makes sure that the attributes are pushed and all signals are connected
		 */
		virtual void registered() override;

		// Slot declarations
		NSLOT(componentAdded, Object&, onAdded)
		NSLOT(showWindow, const SignalAttribute&, onShowWindow)
		NSLOT(hideWindow, const SignalAttribute&, onHideWindow)
		NSLOT(titleChanged, const std::string&, onTitleChanged)
		NSLOT(positionChanged, const glm::ivec2&, onPositionChanged)
		NSLOT(sizeChanged, const glm::ivec2&, onSizeChanged)
		NSLOT(syncChanged, const bool&, onSyncChanged)


	private:
		/**
		 * Pointer to the window that this component manages
		 * This object is set when the component is registered with the
		 * render service
		 */
		std::unique_ptr<RenderWindow> mWindow = nullptr;

		// Settings used when constructing the window
		RenderWindowSettings mSettings;
	};
}

RTTI_DECLARE(nap::RenderWindowComponent)