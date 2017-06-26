#include "windowresource.h"

RTTI_BEGIN_CLASS(nap::WindowResource)
RTTI_END_CLASS

namespace nap
{
	void WindowResource::addEvent(EventPtr inEvent)
	{
		mEvents.emplace_back(std::move(inEvent));
	}

	void WindowResource::processEvents()
	{
		for (auto& event : mEvents)
		{
			onEvent(*event);
		}

		mEvents.clear();
	}
}