#include "renderwindowcomponent.h"

namespace nap
{
	// Constructor
	RenderWindowComponent::RenderWindowComponent()
	{
		// When added request a window
		added.connect(componentAdded);

		// When removed destroy a window
		removed.connect(componentRemoved);

		// When visibility changes, hide / show
		visible.valueChangedSignal.connect(visibilityChanged);

		// Enable edit-ability
		visible.setFlag(Editable, true);
	}


	void RenderWindowComponent::createWindow()
	{
		auto service = getRoot()->getCore().getOrCreateService<RenderService>();
		mWindow = service->getWindow();
	}


	void RenderWindowComponent::destroyWindow()
	{
		auto service = getRoot()->getCore().getOrCreateService<RenderService>();
		service->destroyWindow(mWindow);
	}


	// Hide / Show window
	void RenderWindowComponent::onVisibilityChanged(const bool& value)
	{
		if (value)
		{
			showWindow();
			return;
		}
		hideWindow();
	}

}

RTTI_DEFINE(nap::RenderWindowComponent)


