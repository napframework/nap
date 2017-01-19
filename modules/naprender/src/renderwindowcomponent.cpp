#include "renderwindowcomponent.h"

namespace nap
{
	// Convert this object to an opengl settings container
	opengl::WindowSettings RenderWindowSettings::toGLSettings()
	{
		opengl::WindowSettings settings;
		settings.borderless = this->borderless.getValue();
		settings.height = this->size.getValue().y;
		settings.width = this->size.getValue().x;
		settings.resizable = this->resizable.getValue();
		settings.x = this->position.getValue().x;
		settings.y = this->position.getValue().y;
		settings.title = this->title.getValue();
		return settings;
	}


	// Constructor
	RenderWindowComponent::RenderWindowComponent()
	{
		// When added request a window
		added.connect(componentAdded);

		// When visibility changes, hide / show
		show.signal.connect(showWindow);
		hide.signal.connect(hideWindow);

		// Add settings
		RenderWindowSettings& window_settings = addChild<RenderWindowSettings>("settings");
		window_settings.setFlag(nap::ObjectFlag::Removable, false);
		settings.setTarget(window_settings);
	}


	// Creates the window and spawns it
	void RenderWindowComponent::onAdded(Object& parent)
	{
		nap::Logger::info("Adding window: %s", settings.getTarget<RenderWindowSettings>()->title.getValue().c_str());
	}


	// Shows the window, constructs one if necessary
	void RenderWindowComponent::onShowWindow(const SignalAttribute& signal)
	{
		opengl::showWindow(*mWindow);
	}


	// Hides the window
	void RenderWindowComponent::onHideWindow(const SignalAttribute& signal)
	{
		opengl::hideWindow(*mWindow);
	}
}

RTTI_DEFINE(nap::RenderWindowComponent)
RTTI_DEFINE(nap::RenderWindowSettings)


