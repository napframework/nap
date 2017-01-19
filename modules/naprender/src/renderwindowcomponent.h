#pragma once

// Local Includes
#include "renderservice.h"
#include "renderattributes.h"

// External Includes
#include <nap.h>

namespace nap
{
	// Forward Declares
	class RenderWindowComponent;

	/**
	 * Holds all window launch settings
	 * Note that this object is only used when constructing the window
	 */
	class RenderWindowSettings : public AttributeObject
	{
		friend class RenderWindowComponent;
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)

	public:
		RenderWindowSettings()  = default;
		~RenderWindowSettings() = default;

		Attribute<bool> borderless =		{ this, "Borderless", false };
		Attribute<bool> resizable =			{ this, "Resizable", true };

		/**
		 * Converts the current settings to GL compatible settings
		 * @return an opengl window settings container based on this object
		 */
		opengl::WindowSettings toGLSettings();
	};


	/**
	 * 3D render window. Creating a window requests a new 3d window
	 * with context from the Render Service. Every window is associated
	 * with it's own drawing context. Note that this window does not own
	 * the OpenGL render window, it acts as an interface to the window managed 
	 * by the service
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
		SignalAttribute show { this, "show" };

		/**
		 * Hides the window
		 */
		SignalAttribute hide { this, "hide" };

		/**
		 * Render signal, emitted every render iteration
		 * Connect to this signal to render objects to the context
		 * associated with this window. 
		 */
		SignalAttribute render { this, "render" };

		/**
		 * Link to settings associated with this window
		 * These settings are only used on construction of the window
		 */
		ObjectLinkAttribute constructionSettings =		{ this, "settings", RTTI_OF(RenderWindowSettings) };

		/**
		 * @return if the component manages a window. 
		 * If show hasn't been called this call will resolve to false
		 */
		bool hasWindow() const							{ return mWindow != nullptr; }

		/**
		 * Swaps window buffers
		 */
		void swap() const								{ opengl::swap(*mWindow); }

		/**
		 * These attributes only work when the window has 
		 * been registered with the render service
		 */
		Attribute<glm::ivec2> position					{ this, "Position", { 256, 256 } };
		Attribute<glm::ivec2> size						{ this, "Size", {512, 512 } };
		Attribute<std::string> title					{ this, "Title", "RenderWindow" };

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


	private:
		// Window used for rendering
		std::unique_ptr<opengl::Window> mWindow = nullptr;		// Window used for rendering
	};
}

RTTI_DECLARE(nap::RenderWindowComponent)
RTTI_DECLARE(nap::RenderWindowSettings)