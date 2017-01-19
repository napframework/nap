#include "renderwindowcomponent.h"

namespace nap
{
	// Convert this object to an opengl settings container
	opengl::WindowSettings RenderWindowSettings::toGLSettings()
	{
		opengl::WindowSettings settings;
		settings.borderless = this->borderless.getValue();
		settings.height = 512;
		settings.width = 512;
		settings.resizable = this->resizable.getValue();
		settings.x = 256;
		settings.y = 256;
		settings.title = "RenderWindow";
		return settings;
	}


	// Constructor
	RenderWindowComponent::RenderWindowComponent()
	{
		// When added request a window
		added.connect(componentAdded);

		// Add initialization settings
		RenderWindowSettings& window_settings = addChild<RenderWindowSettings>("settings");

		window_settings.setFlag(nap::ObjectFlag::Removable, false);
		constructionSettings.setTarget(window_settings);
	}


	// Creates the window and spawns it
	void RenderWindowComponent::onAdded(Object& parent)
	{}


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


	// Occurs when the window title changes
	void RenderWindowComponent::onTitleChanged(const std::string& title)
	{
		opengl::setWindowTitle(*mWindow, title.c_str());
	}


	// Set Position
	void RenderWindowComponent::onPositionChanged(const glm::ivec2& position)
	{
		opengl::setWindowPosition(*mWindow, position.x, position.y);
	}

	// Set Size
	void RenderWindowComponent::onSizeChanged(const glm::ivec2& size)
	{
		opengl::setWindowSize(*mWindow, size.x, size.y);
		glViewport(0, 0, size.x, size.y);
	}


	// Pushes attributes to window
	void RenderWindowComponent::registered()
	{
		if (!hasWindow())
		{
			nap::Logger::warn(*this, "unable to connect window parameters, no GL Window");
			return;
		}

		// Install listeners on attribute signals
		position.valueChangedSignal.connect(positionChanged);
		size.valueChangedSignal.connect(sizeChanged);
		title.valueChangedSignal.connect(titleChanged);

		// When visibility changes, hide / show
		show.signal.connect(showWindow);
		hide.signal.connect(hideWindow);

		// Update values that might have been set previously
		onPositionChanged(position.getValue());
		onSizeChanged(size.getValue());
		onTitleChanged(title.getValue());
	}

}

RTTI_DEFINE(nap::RenderWindowComponent)
RTTI_DEFINE(nap::RenderWindowSettings)


