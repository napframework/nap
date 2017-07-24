#include "windowresource.h"

RTTI_BEGIN_BASE_CLASS(nap::Window)
RTTI_END_CLASS

namespace nap
{
	void Window::addEvent(WindowEventPtr inEvent)
	{
		mWindowEvents.emplace_back(std::move(inEvent));
	}

	void Window::processEvents()
	{
		for (auto& event : mWindowEvents)
			onWindowEvent(*event);

		mWindowEvents.clear();
	}
}