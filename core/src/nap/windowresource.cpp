#include "windowresource.h"

RTTI_BEGIN_BASE_CLASS(nap::WindowResource)
RTTI_END_CLASS

namespace nap
{
	void WindowResource::addEvent(WindowEventPtr inEvent)
	{
		mWindowEvents.emplace_back(std::move(inEvent));
	}

	void WindowResource::processEvents()
	{
		for (auto& event : mWindowEvents)
			onWindowEvent(*event);

		mWindowEvents.clear();
	}
}