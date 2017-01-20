#include "renderwindowcomponent.h"

namespace nap
{
	// Constructor
	RenderWindowComponent::RenderWindowComponent()
	{
		// When added request a window
		added.connect(componentAdded);
	}


	// Set settings
	void RenderWindowComponent::setConstructionSettings(const RenderWindowSettings& settings)
	{
		mSettings = settings;
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

	// Turn v-sync on - off
	void RenderWindowComponent::onSyncChanged(const bool& value)
	{
		opengl::setVSync(*mWindow, value);
	}

	// Registers attributes and pushes current state to window
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
		sync.valueChangedSignal.connect(syncChanged);

		// When visibility changes, hide / show
		show.signal.connect(showWindow);
		hide.signal.connect(hideWindow);

		// Push possible values
		onPositionChanged(position.getValue());
		onSizeChanged(size.getValue());
		onTitleChanged(title.getValue());
		onSyncChanged(sync.getValue());
	}

}

RTTI_DEFINE(nap::RenderWindowComponent)
RTTI_DEFINE(nap::RenderWindowSettings)


