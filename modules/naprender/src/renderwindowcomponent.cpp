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
		mWindow->showWindow();
	}


	// Hides the window
	void RenderWindowComponent::onHideWindow(const SignalAttribute& signal)
	{
		mWindow->hideWindow();
	}


	void RenderWindowComponent::onSetActive(const SignalAttribute& signal)
	{
		mWindow->makeCurrent();
	}


	// Occurs when the window title changes
	void RenderWindowComponent::onTitleChanged(const std::string& title)
	{
		mWindow->setTitle(title);
	}


	// Set Position
	void RenderWindowComponent::onPositionChanged(const glm::ivec2& position)
	{
		mWindow->setPosition(position);
	}


	// Set Size
	void RenderWindowComponent::onSizeChanged(const glm::ivec2& size)
	{
		mWindow->setSize(size);
	}


	// Turn v-sync on - off
	void RenderWindowComponent::onSyncChanged(const bool& value)
	{
		mWindow->setSync(value);
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
		activate.signal.connect(setActive);

		// Push possible values
		onPositionChanged(position.getValue());
		onSizeChanged(size.getValue());
		onTitleChanged(title.getValue());
		onSyncChanged(sync.getValue());
	}

}

RTTI_DEFINE(nap::RenderWindowComponent)


